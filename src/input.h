#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <queue>
#include <mutex>
#include <pthread.h>
#include <portmidi.h>
#include "consts.h"
#include "instrument.h"
#include "connection.h"
#include "trigger.h"

using namespace std;

class Input : public Instrument {
public:
  Input(sqlite3_int64 id, PmDeviceID device_id, const char *device_name, const char *name = nullptr);

  virtual bool is_input() { return true; }

  inline bool is_running() { return _running; }
  inline vector<Connection *> &connections() { return _connections; }
  inline vector<Trigger *> &triggers() { return _triggers; }

  void add_connection(Connection *);
  void remove_connection(Connection *);

  void add_trigger(Trigger *);
  void remove_trigger(Trigger *);

  void start();
  void stop();

  void enqueue(PmEvent *, int);
  void read(PmMessage);
  PmMessage message_from_read_queue();
  void stop_read_thread();

protected:
  virtual bool start_midi();

private:
  vector<Connection *> _connections;
  vector<Trigger *> _triggers;
  bool _running;

  vector<Connection *> notes_off_conns[MIDI_CHANNELS][NOTES_PER_CHANNEL];
  vector<Connection *> sustain_off_conns[MIDI_CHANNELS];
  queue<PmMessage> message_queue;
  mutex message_queue_mutex;
  pthread_t read_pthread;

  vector<Connection *> &connections_for_message(PmMessage msg);
};

#endif /* INPUT_H */
