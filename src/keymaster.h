#ifndef KEYMASTER_H
#define KEYMASTER_H

#include <vector>
#include <portmidi.h>
#include "set_list.h"
#include "input.h"
#include "output.h"
#include "message.h"
#include "clock.h"
#include "observer.h"
#include "observable.h"

using namespace std;

class Cursor;

typedef struct ClockContext {
  Song *song;
  bool running;
} ClockContext;

class KeyMaster : public Observer, public Observable {
public:
  KeyMaster();
  ~KeyMaster();

  // ================ accessors ================
  inline vector<Input *> &inputs() { return _inputs; }
  inline vector<Output *> &outputs() { return _outputs; }
  inline SetList *all_songs() { return _set_lists[0]; }
  inline vector<SetList *> &set_lists() { return _set_lists; }
  inline vector<Trigger *> &triggers() { return _triggers; }
  inline vector<Message *> &messages() { return _messages; }
  inline Cursor *cursor() { return _cursor; }
  inline Clock &clock() { return _clock; }
  inline bool is_testing() { return _testing; }
  inline bool is_modified() { return _modified; }

  void add_input(Input *input);
  void add_output(Output *output);

  void add_message(Message *message);
  void remove_message(Message *message);

  void add_trigger(Trigger *trigger);
  void remove_trigger(Trigger *trigger);

  void add_set_list(SetList *set_list);
  void remove_set_list(SetList *set_list);

  void set_testing(bool val) { _testing = val; }

// ================ observer / observable ================
  // Only called by storage after data is loaded or saved.
  void clear_modified();

  // ================ running ================
  void start();
  void stop();

  // ================ clock ================
  void start_clock() { _clock.start(); }
  void continue_clock() { _clock.continue_clock(); }
  void stop_clock() { _clock.stop(); }
  void toggle_clock() { if (_clock.is_running()) _clock.stop(); else _clock.start(); }
  void set_clock_bpm(int bpm) { _clock.set_bpm(bpm); }
  // Get BPM and start/stop from current song and update state of the clock
  void update_clock();

  // ================ initialization ================
  void initialize();
  void load_instruments();

  // ================ movement ================
  void next_patch();
  void prev_patch();
  void next_song();
  void prev_song();

  // ================ going places ================
  void goto_song(Song *song);
  void goto_patch(Patch *patch);

  void goto_song(string name_regex);
  void goto_set_list(string name_regex);

  void jump_to_set_list_index(int i);
  void jump_to_song_index(int i);
  void jump_to_patch_index(int i);

  // ================ doing things ================
  void panic(bool send_notes_off);

  // ================ helpers ================
  void sort_all_songs();

private:
  vector<Input *> _inputs;
  vector<Output *> _outputs;
  vector<Trigger *> _triggers;
  vector<SetList *> _set_lists; // all set lists, including all_songs
  Cursor *_cursor;
  Clock _clock;
  ClockContext _clock_context;
  bool _running;
  bool _testing;
  bool _modified;
  vector<Message *> _messages;

  // ================ initialization ================
  void create_songs();

  // ================ helpers ================
  void patch_stop();
  void patch_start();

  // ================ observer / observable ================
  void update(Observable *o, void *arg);
  // Call the public method clear_modified() to reset _modified to false
  void changed(void *arg = nullptr);
};

KeyMaster *KeyMaster_instance();

#endif /* KEYMASTER_H */
