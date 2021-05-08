#ifndef GUI_H
#define GUI_H

#include "../keymaster.h"
#include "list_window.h"

using namespace std;

class InfoWindow;
class PatchWindow;
class MessagesWindow;
class TriggerWindow;
class MIDIMonitorWindow;
class Window;

typedef enum WindowLayout {
  CURSES_LAYOUT_NORMAL, CURSES_LAYOUT_PLAY
} WindowLayout;

class GUI {
public:
  GUI(KeyMaster * km, WindowLayout layout=CURSES_LAYOUT_NORMAL);
  ~GUI();

  void run();

  void show_message(string);
  void clear_message();
  void clear_message_after(int);
  int clear_message_seconds() { return clear_msg_secs; }
  int clear_message_id() { return clear_msg_id; }

private:
  KeyMaster *km;

  WindowLayout layout;
  Window *message;
  MIDIMonitorWindow *midi_monitor;

  // normal screen
  ListWindow<SetList> *set_lists;
  ListWindow<Song> *set_list;
  ListWindow<Patch> *song;
  PatchWindow *patch;
  MessagesWindow *messages;
  TriggerWindow *trigger;
  InfoWindow *info;

  // play screen
  ListWindow<Patch> *play_song;
  InfoWindow *play_notes;
  PatchWindow *play_patch;

  int clear_msg_secs;
  int clear_msg_id;

  void event_loop();
  void config_curses();
  void create_windows();
  void resize_windows();
  void toggle_view();
  void toggle_midi_monitor();
  void free_windows();
  void refresh_all();
  void set_window_data();
  void set_normal_window_data();
  void set_play_window_data();
  void close_screen();
  void help();
  void load();
  void load(string path);
  void save();
  void save(string path);
  void reload();
  // int max_name_len(vector<Named *> *);
};

GUI *gui_instance();

#endif /* GUI_H */
