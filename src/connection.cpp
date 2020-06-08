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
    xpose(0), pass_through_sysex(false), processing_sysex(false)
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
  if (output_chan != CONNECTION_ALL_CHANNELS) {
    if (prog.bank_msb >= 0)
      midi_out(Pm_Message(CONTROLLER + output_chan, CC_BANK_SELECT_MSB, prog.bank_msb));
    if (prog.bank_lsb >= 0)
      midi_out(Pm_Message(CONTROLLER + output_chan, CC_BANK_SELECT_LSB, prog.bank_lsb));
    if (prog.prog >= 0)
      midi_out(Pm_Message(PROGRAM_CHANGE + output_chan, prog.prog, 0));
  }

  processing_sysex = false;
  input->add_connection(this);
}

void Connection::stop() {
  input->remove_connection(this);
}

// Takes a MIDI message `msg` from an input, processes it, and sends it to
// an output (unless it's been filtered out).
void Connection::midi_in(PmMessage msg) {
  // See if the message should even be processed, or if we should stop here.
  if (!input_channel_ok(msg))
      return;

  int status = Pm_MessageStatus(msg);
  int high_nibble = status & 0xf0;
  int data1 = Pm_MessageData1(msg);
  int data2 = Pm_MessageData2(msg);

  if (status == SYSEX)
    processing_sysex = true;

  // If this is a sysex message, we may or may not filter it out. In any
  // case we pass through any realtime bytes in the sysex message.
  if (processing_sysex) {
    unsigned char data3 = (msg >> 24) & 0xff;
    if (status == EOX || data1 == EOX || data2 == EOX || data3 == EOX ||
         // non-realtime status byte
        (is_status(status) && status < 0xf8 && status != SYSEX))
      processing_sysex = false;

    if (pass_through_sysex) {
      midi_out(msg);
      return;
    }

    // If any of the bytes are realtime bytes AND if we are filtering out
    // sysex, send them.
    if (is_realtime(status))
      midi_out(Pm_Message(status, 0, 0));
    if (is_realtime(data1))
      midi_out(Pm_Message(data1, 0, 0));
    if (is_realtime(data2))
      midi_out(Pm_Message(data2, 0, 0));
    if (is_realtime(data3))
      midi_out(Pm_Message(data3, 0, 0));
    return;
  }

  PmMessage cc_msg;
  Controller *cc;

  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(msg)) {
      data1 += xpose;
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
int Connection::input_channel_ok(PmMessage msg) {
  if (input_chan == CONNECTION_ALL_CHANNELS || processing_sysex)
    return true;

  unsigned char status = Pm_MessageStatus(msg);
  return status >= SYSEX || input_chan == (status & 0x0f);
}

int Connection::inside_zone(PmMessage msg) {
  int note = Pm_MessageData1(msg);
  return note >= zone.low && note <= zone.high;
}

void Connection::midi_out(PmMessage message) {
  PmEvent event = {message, 0};
  output->write(&event, 1);
}
