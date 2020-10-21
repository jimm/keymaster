#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>
#include <portmidi.h>
#include "db_obj.h"
#include "observable.h"
#include "observer.h"
#include "message_filter.h"
#include "controller.h"
#include "curve.h"

using namespace std;

class Input;
class Output;

typedef struct program {
  int bank_msb;
  int bank_lsb;
  int prog;
} program;

typedef struct zone {
  int low;
  int high;
} zone;
  
class Connection : public DBObj, public Observable, public Observer {
public:
  Connection(sqlite3_int64 id, Input *input, int input_chan, Output *output,
             int output_chan);
  ~Connection();

  inline Input *input() { return _input; }
  inline Output *output() { return _output; }
  inline int input_chan() { return _input_chan; }
  inline int output_chan() { return _output_chan; }
  inline int program_bank_msb() { return _prog.bank_msb; }
  inline int program_bank_lsb() { return _prog.bank_lsb; }
  inline int program_prog() { return _prog.prog; }
  inline int zone_low() { return _zone.low; }
  inline int zone_high() { return _zone.high; }
  inline int xpose() { return _xpose; }
  inline Curve *velocity_curve() { return _velocity_curve; }
  inline MessageFilter &message_filter() { return _message_filter; }
  inline bool processing_sysex() { return _processing_sysex; }
  inline bool running() { return _running; }
  inline bool changing_was_running() { return _changing_was_running; }
  inline Controller *cc_map(int i) { return _cc_maps[i]; }

  void set_input(Input *val);
  void set_output(Output *val);
  void set_input_chan(int val);
  void set_output_chan(int val);
  void set_program_bank_msb(int val);
  void set_program_bank_lsb(int val);
  void set_program_prog(int val);
  void set_zone_low(int val);
  void set_zone_high(int val);
  void set_xpose(int val);
  void set_velocity_curve(Curve *val);
  void set_processing_sysex(bool val);
  void set_running(bool val);
  void set_cc_map(int cc_num, Controller *val);

  void start();
  bool is_running();
  void stop();

  // Returns CONNECTION_ALL_CHANNELS if we can't determine what channel to
  // send to (because both input and output don't declare channels). This
  // means that no program change will be sent.
  int program_change_send_channel();

  void begin_changes();
  void end_changes();

  void midi_in(PmMessage msg);

  void set_controller(Controller *controller);
  void remove_cc_num(int cc_num);

  void update(Observable *o, void *arg);

private:
  Input *_input;
  Output *_output;
  int _input_chan;
  int _output_chan;
  struct program _prog;
  struct zone _zone;
  int _xpose;
  Curve *_velocity_curve;
  MessageFilter _message_filter;
  bool _processing_sysex;
  bool _running;
  bool _changing_was_running;
  Controller *_cc_maps[128];

  int input_channel_ok(int status);
  int inside_zone(int note);

  void midi_out(PmMessage msg);
};

#endif /* CONNECTION_H */
