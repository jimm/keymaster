#ifndef MESSAGE_FILTER_H
#define MESSAGE_FILTER_H

class MessageFilter {
public:
  // true means allow, false means filter out

  bool note;                    // both on and off
  bool poly_pressure;
  bool chan_pressure;
  bool program_change;
  bool pitch_bend;
  bool controller;
  bool song_pointer;
  bool song_select;
  bool tune_request;
  bool sysex;
  bool clock;
  bool start_continue_stop;
  bool system_reset;

  MessageFilter();

  bool filter_out(int status, int data1);
};

#endif /* MESSAGE_FILTER_H */