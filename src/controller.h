#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <portmidi.h>
#include "db_obj.h"

class Controller : public DBObj {
public:
  int cc_num;
  int translated_cc_num;
  bool filtered;
  bool pass_through_0;
  bool pass_through_127;

  Controller(sqlite3_int64 id, int cc_num);

  void set_range(bool pass_0, bool pass_127,
                 int min_in, int max_in,
                 int min_out, int max_out);

  // needed by formatter
  int min_in() { return _min_in; }
  int max_in() { return _max_in; }
  int min_out() { return _min_out; }
  int max_out() { return _max_out; }

  // Returns CONTROLLER_BLOCK if nothing to send.
  PmMessage process(PmMessage msg, int output_channel);

private:
  int _min_in;
  int _max_in;
  int _min_out;
  int _max_out;
  float slope;

  int process_data_byte(int val);
};

#endif /* CONTROLLER_H */