#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <set>
#include "error.h"
#include "input.h"
#include "trigger.h"
#include "consts.h"

// 10 milliseconds, in nanoseconds
#define SLEEP_NANOSECS 10000000L
#define INPUT_THREAD_IS_RUNNING (portmidi_pthread != nullptr)

static mutex inputs_mutex;
static set<Input *> inputs;
static pthread_t portmidi_pthread = nullptr;


// For each running input in `inputs`, sees if there is any MIDI data to be
// read. If so, reads as much as is available and tells the input to enqueue
// the data for processing in a separate thread.
//
// When no data is available for any input, sleeps for `SLEEP_NANOSECS`.
void *input_thread(void *_) {
  struct timespec rqtp = {0, SLEEP_NANOSECS};
  PmEvent buf[MIDI_BUFSIZ];

  while (INPUT_THREAD_IS_RUNNING) {
    bool processed_something = false;
    inputs_mutex.lock();
    for (auto& in : inputs) {
      if (!INPUT_THREAD_IS_RUNNING) { // one more chance to stop
        inputs_mutex.unlock();
        return nullptr;
      }
      if (in->running && Pm_Poll(in->stream) == TRUE) {
        int n = Pm_Read(in->stream, buf, MIDI_BUFSIZ);
        if (n > 0) {
          processed_something = true;
          in->enqueue(buf, n);
        }
      }
    }
    inputs_mutex.unlock();
    if (!processed_something && INPUT_THREAD_IS_RUNNING) {
      if (nanosleep(&rqtp, nullptr) == -1)
        return nullptr;
    }
  }
  return nullptr;
}

// While the Input pointed to by `in_voidptr` is running, take PmMessages
// from its input queue and process them. If the queue is empty, sleeps for
// `SLEEP_NANOSECS` before looking in the queue again.
void *read_thread(void *in_voidptr) {
  Input *in = (Input *)in_voidptr;
  struct timespec rqtp = {0, SLEEP_NANOSECS};

  while (in->running) {
    PmMessage msg = in->message_from_read_queue();
    if (msg != 0)
      in->read(msg);
    else {
      if (nanosleep(&rqtp, nullptr) == -1)
        return nullptr;
    }
  }
  return nullptr;
}


Input::Input(sqlite3_int64 id, PmDeviceID device_id, const char *device_name, const char *name)
  : Instrument(id, device_id, device_name, name), running(false), read_pthread(nullptr)
{
}

void Input::add_connection(Connection *conn) {
  connections.push_back(conn);
}

void Input::remove_connection(Connection *conn) {
  for (vector<Connection *>::iterator i = connections.begin(); i != connections.end(); ++i) {
    if (*i == conn) {
      connections.erase(i);
      return;
    }
  }
}

void Input::add_trigger(Trigger *trigger) {
  triggers.push_back(trigger);
}

void Input::remove_trigger(Trigger *trigger) {
  for (vector<Trigger *>::iterator iter = triggers.begin();
       iter != triggers.end();
       ++iter)
    if (trigger == *iter) {
      triggers.erase(iter);
      break;
    }
}

// Lazily starts the `input_thread` if needed. Sets `running` to `true` and
// starts a `read_thread` for this Input.
void Input::start() {
  Instrument::start();
  if (!enabled || !real_port())
    return;

  int status;

  // Not thread safe, but we don't care because this method is called
  // synchronously from a single thread.
  if (portmidi_pthread == nullptr) {
    status = pthread_create(&portmidi_pthread, 0, input_thread, 0);
    if (status != 0) {
      char buf[BUFSIZ];
      sprintf(buf, "error creating global input stream thread %s: %d\n",
              name.c_str(), status);
      error_message(buf);
      exit(1);
    }
  }
  // Don't have to worry about locking the inputs vector since this is only
  // called once in a single thread and the input thread defined above
  // hasn't started yet.
  inputs.insert(this);

  running = true;
  status = pthread_create(&read_pthread, 0, read_thread, this);
  if (status != 0) {
    char buf[BUFSIZ];
    sprintf(buf, "error creating input read thread %s: %d\n",
            name.c_str(), status);
    error_message(buf);
    exit(1);
  }
}

// Sets `running` to `false`, which will cause the `read_thread` for this
// Input to exit. Also removes the Input from `inputs`.
void Input::stop() {
  running = false;
  read_pthread = 0;
  inputs_mutex.lock();
  for (set<Input *>::iterator i = inputs.begin(); i != inputs.end(); ++i) {
    if (*i == this) {
      inputs.erase(i);
      break;
    }
  }
  if (inputs.empty()) {
    // Not really threadsafe, but we don't care because this method is
    // called synchronously from a single thread.
    portmidi_pthread = nullptr;
  }
  inputs_mutex.unlock();

  Instrument::stop();
}

bool Input::start_midi() {
  PmError err = Pm_OpenInput(&stream, device_id, 0, MIDI_BUFSIZ, 0, 0);
  if (err != 0) {
    char buf[BUFSIZ];
    sprintf(buf, "error opening input stream %s: %s\n", name.c_str(),
            Pm_GetErrorText(err));
    error_message(buf);
    return false;
  }

  err = Pm_SetFilter(stream, PM_FILT_ACTIVE); // TODO cmd line option to enable
  if (err != 0) {
    char buf[BUFSIZ];
    sprintf(buf, "error setting PortMidi filter for input %s: %s\n",
            name.c_str(), Pm_GetErrorText(err));
    error_message(buf);
  }

  return true;
}

// Adds all the PmMessages in `events` to our message queue in a thread-safe
// manner.
void Input::enqueue(PmEvent *events, int num) {
  message_queue_mutex.lock();
  for (int i = 0; i < num; ++i)
    message_queue.push(events[i].message);
  message_queue_mutex.unlock();
}

void Input::read(PmMessage msg) {
  if (!enabled && real_port())
    return;

  for (auto& trigger : triggers)
    trigger->signal_message(msg);

  // When testing, remember the messages we've seen. This could be made
  // more efficient by doing a bulk copy before or after this for loop,
  // making sure not to copy over the end of received_messages.
  if (!real_port() && num_io_messages < MIDI_BUFSIZ-1)
    io_messages[num_io_messages++] = msg;

  if (midi_monitor != nullptr)
    midi_monitor->monitor_input(this, msg);

  for (auto &conn : connections_for_message(msg))
    conn->midi_in(msg);
}

// Removes a single PmMessages from our message queue in a thread-safe
// manner and returns it.
PmMessage Input::message_from_read_queue() {
  PmMessage msg = 0;
  message_queue_mutex.lock();
  if (!message_queue.empty()) {
    msg = message_queue.front();
    message_queue.pop();
  }
  message_queue_mutex.unlock();
  return msg;
}

// Return the connections to use for `msg`. Normally it's the same as our
// list of connections. However for every note and sustain on we store those
// connections so we can use them later for the corresponding note and
// sustain offs.
vector<Connection *> &Input::connections_for_message(PmMessage msg) {
  unsigned char status = Pm_MessageStatus(msg);
  unsigned char high_nibble = status & 0xf0;
  unsigned char chan = status & 0x0f;
  unsigned char data1 = Pm_MessageData1(msg);

  // Note off and sustain off messages must be sent to their original
  // connections, so for note on and sustain on messages we store the list
  // of current connections and for off messages we return that list.

  if (high_nibble == NOTE_OFF || (high_nibble == NOTE_ON && Pm_MessageData2(msg) == 0)) {
    if (notes_off_conns[chan][data1].empty())
      return connections;
    return notes_off_conns[chan][data1];
  }

  if (high_nibble == NOTE_ON) {
    notes_off_conns[chan][data1] = connections;
    return notes_off_conns[chan][data1];
  }

  if (high_nibble == CONTROLLER && data1 == CC_SUSTAIN) {
    if (Pm_MessageData2(msg) == 0) {
      if (sustain_off_conns[chan].empty())
        return connections;
      return sustain_off_conns[chan];
    }
    sustain_off_conns[chan] = connections;
    return sustain_off_conns[chan];
  }

  return connections;
}
