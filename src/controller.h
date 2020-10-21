#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <portmidi.h>
#include "db_obj.h"
#include "observable.h"

class Controller : public DBObj, public Observable {
public:
  Controller(sqlite3_int64 id, int cc_num);

  void set_cc_num(int val);
  void set_translated_cc_num(int val);
  void set_filtered(bool val);
  void set_pass_through_0(bool val);
  void set_pass_through_127(bool val);
  void set_range(bool pass_0, bool pass_127,
                 int min_in, int max_in,
                 int min_out, int max_out);

  inline int cc_num() { return _cc_num; }
  inline int translated_cc_num() { return _translated_cc_num; }
  inline int filtered() { return _filtered; }
  inline int pass_through_0() { return _pass_through_0; }
  inline int pass_through_127() { return _pass_through_127; }

  // needed by formatter
  inline int min_in() { return _min_in; }
  inline int max_in() { return _max_in; }
  inline int min_out() { return _min_out; }
  inline int max_out() { return _max_out; }

  // Returns CONTROLLER_BLOCK if nothing to send.
  PmMessage process(PmMessage msg, int output_channel);

private:
  int _cc_num;
  int _translated_cc_num;
  bool _filtered;
  bool _pass_through_0;
  bool _pass_through_127;
  int _min_in;
  int _max_in;
  int _min_out;
  int _max_out;
  float _slope;

  int process_data_byte(int val);
};

#endif /* CONTROLLER_H */
