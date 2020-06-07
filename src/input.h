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
  vector<Connection *> connections;
  vector<Trigger *> triggers;
  bool running;

  Input(int id, const char *name, const char *port_name, int port_num);

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
  vector<Connection *> notes_off_conns[MIDI_CHANNELS][NOTES_PER_CHANNEL];
  vector<Connection *> sustain_off_conns[MIDI_CHANNELS];
  queue<PmMessage> message_queue;
  mutex message_queue_mutex;
  pthread_t read_pthread;

  vector<Connection *> &connections_for_message(PmMessage msg);
};

#endif /* INPUT_H */
