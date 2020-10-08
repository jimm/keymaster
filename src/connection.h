#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>
#include <portmidi.h>
#include "db_obj.h"
#include "message_filter.h"
#include "controller.h"

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

class Connection : public DBObj {
public:
  Input *input;
  Output *output;
  int input_chan;
  int output_chan;
  program prog;
  zone zone;
  int xpose;
  MessageFilter message_filter;
  bool processing_sysex;
  bool running;
  bool changing_was_running;
  Controller *cc_maps[128];

  Connection(sqlite3_int64 id, Input *input, int input_chan, Output *output,
             int output_chan);
  ~Connection();

  void start();
  bool is_running();
  void stop();

  void begin_changes();
  void end_changes();

  void midi_in(PmMessage msg);

  void set_controller(Controller *controller);
  void remove_cc_num(int cc_num);

private:
  int input_channel_ok(int status);
  int inside_zone(int note);

  void midi_out(PmMessage msg);
};

#endif /* CONNECTION_H */
