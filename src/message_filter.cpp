#include "consts.h"
#include "message_filter.h"

// true means allow, false means filter out

MessageFilter::MessageFilter()
  :
  _note(true),                  // both on and off
  _poly_pressure(true),
  _chan_pressure(true),
  _program_change(false),       // filter out by default
  _pitch_bend(true),
  _controller(true),
  _song_pointer(true),
  _song_select(true),
  _tune_request(true),
  _sysex(false),                // filter out by default
  _clock(true),
  _start_continue_stop(true),
  _system_reset(true)
{
}

void MessageFilter::set_note(bool val) {      // both on and off
  if (_note != val) {
    _note = val;
    changed();
  }
}

void MessageFilter::set_poly_pressure(bool val) {
  if (_poly_pressure != val) {
    _poly_pressure = val;
    changed();
  }
}

void MessageFilter::set_chan_pressure(bool val) {
  if (_chan_pressure != val) {
    _chan_pressure = val;
    changed();
  }
}

void MessageFilter::set_program_change(bool val) {
  if (_program_change != val) {
    _program_change = val;
    changed();
  }
}

void MessageFilter::set_pitch_bend(bool val) {
  if (_pitch_bend != val) {
    _pitch_bend = val;
    changed();
  }
}

void MessageFilter::set_controller(bool val) {
  if (_controller != val) {
    _controller = val;
    changed();
  }
}

void MessageFilter::set_song_pointer(bool val) {
  if (_song_pointer != val) {
    _song_pointer = val;
    changed();
  }
}

void MessageFilter::set_song_select(bool val) {
  if (_song_select != val) {
    _song_select = val;
    changed();
  }
}

void MessageFilter::set_tune_request(bool val) {
  if (_tune_request != val) {
    _tune_request = val;
    changed();
  }
}

void MessageFilter::set_sysex(bool val) {
  if (_sysex != val) {
    _sysex = val;
    changed();
  }
}

void MessageFilter::set_clock(bool val) {
  if (_clock != val) {
    _clock = val;
    changed();
  }
}

void MessageFilter::set_start_continue_stop(bool val) {
  if (_start_continue_stop != val) {
    _start_continue_stop = val;
    changed();
  }
}

void MessageFilter::set_system_reset(bool val) {
  if (_system_reset != val) {
    _system_reset = val;
    changed();
  }
}

// Return true if the message for this status should be filtered out.
bool MessageFilter::filter_out(int status, int data1) {
  if (status < 0xf0)
    status = status & 0xf0;
  switch (status) {
    case NOTE_OFF:
    case NOTE_ON:
      return !_note;
    case POLY_PRESSURE:
      return !POLY_PRESSURE;
    case CONTROLLER:
      if (data1 == CC_BANK_SELECT_MSB || data1 == CC_BANK_SELECT_LSB)
        return !_program_change;
      return !_controller;
    case PROGRAM_CHANGE:
      return !_program_change;
    case CHANNEL_PRESSURE:
      return !_chan_pressure;
    case PITCH_BEND:
      return !_pitch_bend;
    case SYSEX:
    case EOX:
      return !_sysex;
    case SONG_POINTER:
      return !_song_pointer;
    case SONG_SELECT:
      return !_song_select;
    case TUNE_REQUEST:
      return !_tune_request;
    case CLOCK:
      return !_clock;
    case START:
    case CONTINUE:
    case STOP:
      return !_start_continue_stop;
    case ACTIVE_SENSE:
      // When we initialize PortMidi we always filter out all active sensing
      // messages. This does the same thing.
      return true;
    case SYSTEM_RESET:
      return !_system_reset;
  default:
    // error, but don't prevent wierdness from happening I suppose
    return false;
  }
}
