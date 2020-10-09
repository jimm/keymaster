#include <stdlib.h>
#include "consts.h"
#include "connection.h"
#include "input.h"
#include "output.h"

#define is_status(b) (((b) & 0x80) == 0x80)
#define is_realtime(b) ((b) >= 0xf8)

Connection::Connection(sqlite3_int64 id, Input *in, int in_chan, Output *out,
                       int out_chan)
  : DBObj(id),
    input(in), input_chan(in_chan),
    output(out), output_chan(out_chan),
    xpose(0), velocity_curve(curve_with_shape(Linear)), processing_sysex(false),
    running(false), changing_was_running(false)
{
  prog.bank_msb = prog.bank_lsb = prog.prog = UNDEFINED;
  zone.low = 0;
  zone.high = 127;
  for (int i = 0; i < 128; ++i)
    cc_maps[i] = nullptr;
}

Connection::~Connection() {
  for (int i = 0; i < 128; ++i)
    if (cc_maps[i] != nullptr)
      delete cc_maps[i];
}

void Connection::start() {
  if (running)
    return;

  // The program output channel is either the output channel if specified or
  // else the output channel if specified. If they are both ALL then we
  // don't know which channel to send the program change on, so we don't
  // send one.
  int chan = program_change_send_channel();
  if (chan != CONNECTION_ALL_CHANNELS) {
    if (prog.bank_msb >= 0)
      midi_out(Pm_Message(CONTROLLER + chan, CC_BANK_SELECT_MSB, prog.bank_msb));
    if (prog.bank_lsb >= 0)
      midi_out(Pm_Message(CONTROLLER + chan, CC_BANK_SELECT_LSB, prog.bank_lsb));
    if (prog.prog >= 0)
      midi_out(Pm_Message(PROGRAM_CHANGE + chan, prog.prog, 0));
  }

  processing_sysex = false;
  input->add_connection(this);
  running = true;
}

bool Connection::is_running() {
  return running;
}

void Connection::stop() {
  if (!running)
    return;
  input->remove_connection(this);
  running = false;
}

// Returns the channel that we should send the initial bank/program change
// messages. If we can't determine that (both input and output channels are
// CONNECTION_ALL_CHANNELS) then return CONNECTION_ALL_CHANNELS.
int Connection::program_change_send_channel() {
  if (output_chan != CONNECTION_ALL_CHANNELS)
    return output_chan;
  return input_chan;
}

// Call this when a Connection is being edited so that it can restart itself
// if it is running.
void Connection::begin_changes() {
  changing_was_running = is_running();
  if (changing_was_running)
    stop();
}

// Call this when done making changes so the Connection can restart itself
// if it was running.
void Connection::end_changes() {
  if (changing_was_running) {
    start();
    changing_was_running = false;
  }
}

// Takes a MIDI message `msg` from an input, processes it, and sends it to
// an output (unless it's been filtered out).
void Connection::midi_in(PmMessage msg) {
  int status = Pm_MessageStatus(msg);

  // See if the message should even be processed, or if we should stop here.
  if (!input_channel_ok(status))
      return;

  int high_nibble = status & 0xf0;
  int data1 = Pm_MessageData1(msg);
  int data2 = Pm_MessageData2(msg);

  if (status == SYSEX)
    processing_sysex = true;

  // Grab filter boolean for this status. If we're inside a sysex message,
  // we need use SYSEX as the filter status, not the first byte of this
  // message.
  bool filter_this_status = message_filter.filter_out(processing_sysex ? SYSEX : status, data1);

  // Return if we're filtering this message, exept if we're starting or in
  // sysex. In that case we need to keep going because we need to process
  // realtime bytes within the sysex.
  if (!processing_sysex && filter_this_status)
    return;

  // If this is a sysex message, we may or may not filter it out. In any
  // case we pass through any realtime bytes in the sysex message.
  if (processing_sysex) {
    unsigned char data3 = (msg >> 24) & 0xff;
    if (status == EOX || data1 == EOX || data2 == EOX || data3 == EOX ||
         // non-realtime status byte
        (is_status(status) && status < 0xf8 && status != SYSEX))
      processing_sysex = false;

    if (!filter_this_status) {
      midi_out(msg);
      return;
    }

    // If any of the bytes are realtime bytes AND if we are filtering out
    // sysex, send them anyway.
    if (is_realtime(status) && !message_filter.filter_out(status, 0))
      midi_out(Pm_Message(status, 0, 0));
    if (is_realtime(data1) && !message_filter.filter_out(data1, 0))
      midi_out(Pm_Message(data1, 0, 0));
    if (is_realtime(data2) && !message_filter.filter_out(data2, 0))
      midi_out(Pm_Message(data2, 0, 0));
    if (is_realtime(data3) && !message_filter.filter_out(data3, 0))
      midi_out(Pm_Message(data3, 0, 0));
    return;
  }

  PmMessage cc_msg;
  Controller *cc;

  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(data1)) {
      data1 += xpose;
      data2 = velocity_curve->curve[data2];
      if (data1 >= 0 && data1 <= 127) {
        if (output_chan != CONNECTION_ALL_CHANNELS)
          status = high_nibble + output_chan;
        midi_out(Pm_Message(status, data1, data2));
      }
    }
    break;
  case CONTROLLER:
    cc = cc_maps[data1];
    if (cc != nullptr) {
      cc_msg = cc->process(msg, output_chan);
      if (cc_msg != CONTROLLER_BLOCK)
        midi_out(cc_msg);
    }
    else {
      if (output_chan != CONNECTION_ALL_CHANNELS)
        status = high_nibble + output_chan;
      midi_out(Pm_Message(status, data1, data2));
    }
    break;
  case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case PITCH_BEND:
    if (output_chan != CONNECTION_ALL_CHANNELS)
      status = high_nibble + output_chan;
    midi_out(Pm_Message(status, data1, data2));
    break;
  default:
    midi_out(msg);
    break;
  }
}

void Connection::set_controller(Controller *controller) {
  int cc_num = controller->cc_num;
  if (cc_maps[cc_num] != nullptr && cc_maps[cc_num] != controller)
    remove_cc_num(cc_num);
  cc_maps[cc_num] = controller;
}

void Connection::remove_cc_num(int cc_num) {
  delete cc_maps[cc_num];
  cc_maps[cc_num] = nullptr;
}

// Returns `true` if any one of the following are true:
// - we accept any input channel
// - it's a system message, not a channel message
// - the input channel matches our selected `input_chan`
int Connection::input_channel_ok(int status) {
  if (input_chan == CONNECTION_ALL_CHANNELS || processing_sysex)
    return true;

  return status >= SYSEX || input_chan == (status & 0x0f);
}

int Connection::inside_zone(int note) {
  return note >= zone.low && note <= zone.high;
}

void Connection::midi_out(PmMessage message) {
  PmEvent event = {message, 0};
  output->write(&event, 1);
}
