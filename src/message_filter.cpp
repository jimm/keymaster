#include "consts.h"
#include "message_filter.h"

// true means allow, false means filter out

MessageFilter::MessageFilter()
  :
  note(true),                   // both on and off
  poly_pressure(true),
  chan_pressure(true),
  program_change(false),        // filter out by default
  pitch_bend(true),
  controller(true),
  song_pointer(true),
  song_select(true),
  tune_request(true),
  sysex(false),                 // filter out by default
  clock(true),
  start_continue_stop(true),
  system_reset(true)
{
}

// Return true if the message for this status should be filtered out.
bool MessageFilter::filter_out(int status, int data1) {
  if (status < 0xf0)
    status = status & 0xf0;
  switch (status) {
    case NOTE_OFF:
    case NOTE_ON:
      return !note;
    case POLY_PRESSURE:
      return !POLY_PRESSURE;
    case CONTROLLER:
      if (data1 == CC_BANK_SELECT_MSB || data1 == CC_BANK_SELECT_LSB)
        return !program_change;
      return !controller;
    case PROGRAM_CHANGE:
      return !program_change;
    case CHANNEL_PRESSURE:
      return !chan_pressure;
    case PITCH_BEND:
      return !pitch_bend;
    case SYSEX:
    case EOX:
      return !sysex;
    case SONG_POINTER:
      return !song_pointer;
    case SONG_SELECT:
      return !song_select;
    case TUNE_REQUEST:
      return !tune_request;
    case CLOCK:
      return !clock;
    case START:
    case CONTINUE:
    case STOP:
      return !start_continue_stop;
    case ACTIVE_SENSE:
      // When we initialize PortMidi we always filter out all active sensing
      // messages. This does the same thing.
      return true;
    case SYSTEM_RESET:
      return !system_reset;
  default:
    // error, but don't prevent wierdness from happening I suppose
    return false;
  }
}
