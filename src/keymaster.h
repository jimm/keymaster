#ifndef KEYMASTER_H
#define KEYMASTER_H

#include <vector>
#include <portmidi.h>
#include "set_list.h"
#include "input.h"
#include "output.h"
#include "message.h"
#include "clock.h"

using namespace std;

class Cursor;

class KeyMaster {
public:
  vector<Input *> inputs;
  vector<Output *> outputs;
  vector<Trigger *> triggers;
  SetList *all_songs;
  vector<SetList *> set_lists;
  Cursor *cursor;
  Clock clock;
  bool running;
  bool testing;
  vector<Message *> messages;
  string loaded_from_file;

  KeyMaster();
  ~KeyMaster();

  // ================ running ================
  void start();
  void stop();

  // ================ clock ================
  void start_clock() { clock.start(); }
  void stop_clock() { clock.stop(); }
  void toggle_clock() { if (is_clock_running()) clock.stop(); else clock.start(); }
  void set_clock_bpm(int bpm) { clock.set_bpm(bpm); }
  bool is_clock_running() { return clock.is_running(); }
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
  // ================ initialization ================
  void create_songs();

  // ================ clock ================
};

KeyMaster *KeyMaster_instance();

#endif /* KEYMASTER_H */
