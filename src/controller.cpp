#include <math.h>
#include "consts.h"
#include "controller.h"

Controller::Controller(sqlite3_int64 id, int num)
  : DBObj(id),
    _cc_num(num),
    _translated_cc_num(num),
    _pass_through_0(true), _pass_through_127(true),
    _min_in(0), _max_in(127),
    _min_out(0), _max_out(127),
    _filtered(false)
{
}

void Controller::set_cc_num(int val) {
  if (_cc_num != val) {
    _cc_num = val;
    changed();
  }
}

void Controller::set_translated_cc_num(int val) {
  if (_translated_cc_num != val) {
    _translated_cc_num = val;
    changed();
  }
}

void Controller::set_filtered(bool val) {
  if (_filtered != val) {
    _filtered = val;
    changed();
  }
}

void Controller::set_pass_through_0(bool val) {
  if (_pass_through_0 != val) {
    _pass_through_0 = val;
    changed();
  }
}

void Controller::set_pass_through_127(bool val) {
  if (_pass_through_127 != val) {
    _pass_through_127 = val;
    changed();
  }
}

void Controller::set_range(bool pass_0, bool pass_127,
                           int min_in, int max_in,
                           int min_out, int max_out)
{
  bool modified = false;
  if (_pass_through_0 != pass_0) {
    _pass_through_0 = pass_0;
    modified = true;
  }
  if (_pass_through_127 != pass_127) {
    _pass_through_127 = pass_127;
    modified = true;
  }
  if (_min_in != min_in) {
    _min_in = min_in;
    modified = true;
  }
  if (_min_out != min_out) {
    _min_out = min_out;
    modified = true;
  }
  if (_max_in != max_in) {
    _max_in = max_in;
    modified = true;
  }
  if (_max_out != max_out) {
    _max_out = max_out;
    modified = true;
  }

  float new_slope = ((float)_max_out - (float)_min_out) / ((float)_max_in - (float)_min_in);
  if (_slope != new_slope) {
    _slope = new_slope;
    modified = true;
  }
  if (modified)
    changed();
}

PmMessage Controller::process(PmMessage msg, int output_chan) {
  if (_filtered)
    return CONTROLLER_BLOCK;

  int chan = (output_chan != CONNECTION_ALL_CHANNELS)
    ? output_chan
    : (Pm_MessageStatus(msg) & 0x0f);
  int data2 = Pm_MessageData2(msg);

  int new_val = process_data_byte(data2);
  if (new_val == CONTROLLER_BLOCK)
    return CONTROLLER_BLOCK;
  return Pm_Message(CONTROLLER + chan, _translated_cc_num, new_val);
}

int Controller::process_data_byte(int val) {
  // simplest case: no change in value
  if (_min_in == 0 && _max_in == 127 &&
      _min_out == 0 && _max_out == 127)
    return val;

  // pass-through 0 or 127 values if requested
  if (val == 0 && _pass_through_0)
    return 0;
  if (val == 127 && _pass_through_127)
    return 127;

  // input value out of range, filter out
  if (val < _min_in || val > _max_in)
    return CONTROLLER_BLOCK;

  return _min_out + floor((_slope * (val - _min_in)) + 0.5);
}
