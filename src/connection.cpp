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
    _input(in), _input_chan(in_chan),
    _output(out), _output_chan(out_chan),
    _xpose(0), _velocity_curve(nullptr),
    _processing_sysex(false),
    _running(false), _changing_was_running(false)
{
  _prog.bank_msb = _prog.bank_lsb = _prog.prog = UNDEFINED;
  _zone.low = 0;
  _zone.high = 127;
  for (int i = 0; i < 128; ++i)
    _cc_maps[i] = nullptr;
}

Connection::~Connection() {
  for (int i = 0; i < 128; ++i)
    if (_cc_maps[i] != nullptr)
      delete _cc_maps[i];
}

void Connection::set_input(Input *val) {
  if (_input != val) {
    _input = val;
    changed();
  }
}

void Connection::set_output(Output *val) {
  if (_output != val) {
    _output = val;
    changed();
  }
}

void Connection::set_input_chan(int val) {
  if (_input_chan != val) {
    _input_chan = val;
    changed();
  }
}

void Connection::set_output_chan(int val) {
  if (_output_chan != val) {
    _output_chan = val;
    changed();
  }
}

void Connection::set_program_bank_msb(int val) {
  if (_prog.bank_msb != val) {
    _prog.bank_msb = val;
    changed();
  }
}

void Connection::set_program_bank_lsb(int val) {
  if (_prog.bank_lsb != val) {
    _prog.bank_lsb = val;
    changed();
  }
}

void Connection::set_program_prog(int val) {
  if (_prog.prog != val) {
    _prog.prog = val;
    changed();
  }
}

void Connection::set_zone_low(int val) {
  if (_zone.low != val) {
    _zone.low = val;
    changed();
  }
}

void Connection::set_zone_high(int val) {
  if (_zone.high != val) {
    _zone.high = val;
    changed();
  }
}

void Connection::set_xpose(int val) {
  if (_xpose != val) {
    _xpose = val;
    changed();
  }
}

void Connection::set_velocity_curve(Curve *val) {
  if (_velocity_curve != val) {
    _velocity_curve = val;
    changed();
  }
}

void Connection::set_processing_sysex(bool val) {
  if (_processing_sysex != val) {
    _processing_sysex = val;
    changed();
  }
}

void Connection::set_running(bool val) {
  if (_running != val) {
    _running = val;
    changed();
  }
}

void Connection::set_cc_map(int cc_num, Controller *val) {
  val->add_observer(this);
  if (_cc_maps[cc_num] != val) {
    _cc_maps[cc_num] = val;
    changed();
  }
}

void Connection::start() {
  if (_running)
    return;

  // The program output channel is either the output channel if specified or
  // else the output channel if specified. If they are both ALL then we
  // don't know which channel to send the program change on, so we don't
  // send one.
  int chan = program_change_send_channel();
  if (chan != CONNECTION_ALL_CHANNELS) {
    if (_prog.bank_msb >= 0)
      midi_out(Pm_Message(CONTROLLER + chan, CC_BANK_SELECT_MSB, _prog.bank_msb));
    if (_prog.bank_lsb >= 0)
      midi_out(Pm_Message(CONTROLLER + chan, CC_BANK_SELECT_LSB, _prog.bank_lsb));
    if (_prog.prog >= 0)
      midi_out(Pm_Message(PROGRAM_CHANGE + chan, _prog.prog, 0));
  }

  _processing_sysex = false;
  _input->add_connection(this);
  _running = true;
}

bool Connection::is_running() {
  return _running;
}

void Connection::stop() {
  if (!_running)
    return;
  _input->remove_connection(this);
  _running = false;
}

// Returns the channel that we should send the initial bank/program change
// messages. If we can't determine that (both input and output channels are
// CONNECTION_ALL_CHANNELS) then return CONNECTION_ALL_CHANNELS.
int Connection::program_change_send_channel() {
  if (_output_chan != CONNECTION_ALL_CHANNELS)
    return _output_chan;
  return _input_chan;
}

// Call this when a Connection is being edited so that it can restart itself
// if it is _running.
void Connection::begin_changes() {
  _changing_was_running = is_running();
  if (_changing_was_running)
    stop();
}

// Call this when done making changes so the Connection can restart itself
// if it was _running.
void Connection::end_changes() {
  if (_changing_was_running) {
    start();
    _changing_was_running = false;
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
    _processing_sysex = true;

  // Grab filter boolean for this status. If we're inside a sysex message,
  // we need use SYSEX as the filter status, not the first byte of this
  // message.
  bool filter_this_status = _message_filter.filter_out(_processing_sysex ? SYSEX : status, data1);

  // Return if we're filtering this message, exept if we're starting or in
  // sysex. In that case we need to keep going because we need to process
  // realtime bytes within the sysex.
  if (!_processing_sysex && filter_this_status)
    return;

  // If this is a sysex message, we may or may not filter it out. In any
  // case we pass through any realtime bytes in the sysex message.
  if (_processing_sysex) {
    unsigned char data3 = (msg >> 24) & 0xff;
    if (status == EOX || data1 == EOX || data2 == EOX || data3 == EOX ||
         // non-realtime status byte
        (is_status(status) && status < 0xf8 && status != SYSEX))
      _processing_sysex = false;

    if (!filter_this_status) {
      midi_out(msg);
      return;
    }

    // If any of the bytes are realtime bytes AND if we are filtering out
    // sysex, send them anyway.
    if (is_realtime(status) && !_message_filter.filter_out(status, 0))
      midi_out(Pm_Message(status, 0, 0));
    if (is_realtime(data1) && !_message_filter.filter_out(data1, 0))
      midi_out(Pm_Message(data1, 0, 0));
    if (is_realtime(data2) && !_message_filter.filter_out(data2, 0))
      midi_out(Pm_Message(data2, 0, 0));
    if (is_realtime(data3) && !_message_filter.filter_out(data3, 0))
      midi_out(Pm_Message(data3, 0, 0));
    return;
  }

  PmMessage cc_msg;
  Controller *cc;

  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(data1)) {
      data1 += _xpose;
      if (_velocity_curve != nullptr)
        data2 = _velocity_curve->curve[data2];
      if (data1 >= 0 && data1 <= 127) {
        if (_output_chan != CONNECTION_ALL_CHANNELS)
          status = high_nibble + _output_chan;
        midi_out(Pm_Message(status, data1, data2));
      }
    }
    break;
  case CONTROLLER:
    cc = _cc_maps[data1];
    if (cc != nullptr) {
      cc_msg = cc->process(msg, _output_chan);
      if (cc_msg != CONTROLLER_BLOCK)
        midi_out(cc_msg);
    }
    else {
      if (_output_chan != CONNECTION_ALL_CHANNELS)
        status = high_nibble + _output_chan;
      midi_out(Pm_Message(status, data1, data2));
    }
    break;
  case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case PITCH_BEND:
    if (_output_chan != CONNECTION_ALL_CHANNELS)
      status = high_nibble + _output_chan;
    midi_out(Pm_Message(status, data1, data2));
    break;
  default:
    midi_out(msg);
    break;
  }
}

void Connection::set_controller(Controller *controller) {
  controller->add_observer(this);
  int cc_num = controller->cc_num();
  if (_cc_maps[cc_num] != nullptr && _cc_maps[cc_num] != controller)
    remove_cc_num(cc_num);
  _cc_maps[cc_num] = controller;
  changed();
}

void Connection::remove_cc_num(int cc_num) {
  if (_cc_maps[cc_num] != nullptr) {
    _cc_maps[cc_num]->remove_observer(this);
    delete _cc_maps[cc_num];
    _cc_maps[cc_num] = nullptr;
    changed();
  }
}

// Returns `true` if any one of the following are true:
// - we accept any input channel
// - it's a system message, not a channel message
// - the input channel matches our selected `input_chan`
int Connection::input_channel_ok(int status) {
  if (_input_chan == CONNECTION_ALL_CHANNELS || _processing_sysex)
    return true;

  return status >= SYSEX || _input_chan == (status & 0x0f);
}

int Connection::inside_zone(int note) {
  return note >= _zone.low && note <= _zone.high;
}

void Connection::midi_out(PmMessage message) {
  PmEvent event = {message, 0};
  _output->write(&event, 1);
}

void Connection::update(Observable *o, void *arg) {
  changed();
}
