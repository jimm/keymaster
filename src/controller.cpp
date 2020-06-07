#include <math.h>
#include "consts.h"
#include "controller.h"

Controller::Controller(sqlite3_int64 id, int num)
  : DBObj(id),
    cc_num(num),
    translated_cc_num(num),
    pass_through_0(true), pass_through_127(true),
    _min_in(0), _max_in(127),
    _min_out(0), _max_out(127),
    filtered(false)
{
}

void Controller::set_range(bool pass_0, bool pass_127,
                           int min_in, int max_in,
                           int min_out, int max_out)
{
  pass_through_0 = pass_0;
  pass_through_127 = pass_127;
  _min_in = min_in;
  _max_in = max_in;
  _min_out = min_out;
  _max_out = max_out;
  slope = ((float)_max_out - (float)_min_out) / ((float)_max_in - (float)_min_in);
}

PmMessage Controller::process(PmMessage msg, int output_chan) {
  if (filtered)
    return CONTROLLER_BLOCK;

  int chan = (output_chan != CONNECTION_ALL_CHANNELS)
    ? output_chan
    : (Pm_MessageStatus(msg) & 0x0f);
  int data2 = Pm_MessageData2(msg);

  int new_val = process_data_byte(data2);
  if (new_val == CONTROLLER_BLOCK)
    return CONTROLLER_BLOCK;
  return Pm_Message(CONTROLLER + chan, translated_cc_num, new_val);
}

int Controller::process_data_byte(int val) {
  // simplest case: no change in value
  if (_min_in == 0 && _max_in == 127 &&
      _min_out == 0 && _max_out == 127)
    return val;

  // pass-through 0 or 127 values if requested
  if (val == 0 && pass_through_0)
    return 0;
  if (val == 127 && pass_through_127)
    return 127;

  // input value out of range, filter out
  if (val < _min_in || val > _max_in)
    return CONTROLLER_BLOCK;

  return _min_out + floor((slope * (val - _min_in)) + 0.5);
}
