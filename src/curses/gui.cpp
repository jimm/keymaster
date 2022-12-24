#include <sstream>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "gui.h"
#include "../consts.h"
#include "../keymaster.h"
#include "../storage.h"
#include "geometry.h"
#include "help_window.h"
#include "info_window.h"
#include "list_window.h"
#include "list_window.cpp"      // for templates
#include "patch_window.h"
#include "prompt_window.h"
#include "midi_monitor_window.h"
#include "../cursor.h"


static GUI *g_instance = 0;

GUI *gui_instance() {
  return g_instance;
}

template <class T>
int max_name_len(vector<T *> *list) {
  int maxlen = 0;
  for (auto& named : *list) {
    int len = named->name().length();
    if (len > maxlen)
      maxlen = len;
  }
  return maxlen;
}

GUI::GUI(KeyMaster *kmaster, WindowLayout wlayout)
  : km(kmaster), layout(wlayout), midi_monitor(nullptr), clear_msg_id(0)
{
  g_instance = this;
}

GUI::~GUI() {
  if (g_instance == this)
    g_instance = 0;
}

void GUI::run() {
  config_curses();
  create_windows();
  event_loop();
  clear();
  refresh();
  close_screen();
  free_windows();
}

void GUI::event_loop() {
  bool done = FALSE;
  int ch, prev_cmd = 0;
  PromptWindow *pwin;
  string name_regex;

  while (!done) {
    refresh_all();
    ch = getch();
    switch (ch) {
    case 'j': case KEY_DOWN: case ' ':
      km->next_patch();
      break;
    case 'k': case KEY_UP:
      km->prev_patch();
      break;
    case 'n': case KEY_RIGHT:
      km->next_song();
      break;
    case 'p': case KEY_LEFT:
      km->prev_song();
      break;
    case 'g':
      pwin = new PromptWindow("Go To Song");
      name_regex = pwin->gets();
      delete pwin;
      if (name_regex.length() > 0)
        km->goto_song(name_regex);
      break;
    case 't':
      pwin = new PromptWindow("Go To Song List");
      name_regex = pwin->gets();
      delete pwin;
      if (name_regex.length() > 0)
        km->goto_set_list(name_regex);
      break;
    case 'h': case '?':
      help();
      break;
    case '\e':                  /* escape */
      if (midi_monitor != nullptr)
        toggle_midi_monitor();
      else {
        show_message("Sending panic...");
        km->panic(prev_cmd == '\e');
        show_message("Panic sent");
        clear_message_after(5);
      }
      break;
    case 'l':
      load();
      break;
    case 's':
      save();
      break;
    case 'r':
      reload();
      break;
    case 'v':
      toggle_view();
      break;
    case 'm':
      toggle_midi_monitor();
      break;
    case 'c':
      toggle_midi_monitor_show_clock();
      break;
    case 'q':
      done = TRUE;
      break;
    case KEY_RESIZE:
      resize_windows();
      break;
    }
    prev_cmd = ch;

    // TODO messages and code keys
    /* msg_name = @km->message_bindings[ch]; */
    /* @km->send_message(msg_name) if msg_name; */
    /* code_key = @km->code_bindings[ch]; */
    /* code_key.call if code_key; */
  }
}

void GUI::config_curses() {
  setenv("ESCDELAY", "25", 1);
  initscr();
  cbreak();                     /* unbuffered input */
  noecho();                     /* do not show typed keys */
  keypad(stdscr, true);         /* enable arrow keys and window resize as keys */
  nl();                         /* return key => newline, \n => \r\n */
  curs_set(0);                  /* cursor: 0 = invisible, 1 = normal */
}

void GUI::create_windows() {
  set_lists = new ListWindow<SetList>(geom_set_lists_rect(), 0);
  set_list = new ListWindow<Song>(geom_set_list_rect(), "Song List");
  song = new ListWindow<Patch>(geom_song_rect(), "Song");
  patch = new PatchWindow(geom_patch_rect(), "Patch",
                          max_name_len(&km->inputs()), max_name_len(&km->outputs()));
  message = new Window(geom_message_rect(), "");
  messages = new ListWindow<Message>(geom_messages_rect(), 0);
  triggers = new ListWindow<Trigger>(geom_triggers_rect(), 0);
  info = new InfoWindow(geom_info_rect(), "");

  play_song = new ListWindow<Patch>(geom_play_song_rect(), "");
  play_notes = new InfoWindow(geom_play_notes_rect(), "Notes / Help");
  play_patch = new PatchWindow(geom_play_patch_rect(), "Patch",
                               max_name_len(&km->inputs()), max_name_len(&km->outputs()));

  scrollok(stdscr, false);
  scrollok(message->win, false);
}

void GUI::resize_windows() {
  set_lists->move_and_resize(geom_set_lists_rect());
  set_list->move_and_resize(geom_set_list_rect());
  song->move_and_resize(geom_song_rect());
  patch->move_and_resize(geom_patch_rect());
  message->move_and_resize(geom_message_rect());
  messages->move_and_resize(geom_messages_rect());
  triggers->move_and_resize(geom_triggers_rect());
  info->move_and_resize(geom_info_rect());

  play_song->move_and_resize(geom_play_song_rect());
  play_notes->move_and_resize(geom_play_notes_rect());
  play_patch->move_and_resize(geom_patch_rect());

  if (midi_monitor != nullptr)
    midi_monitor->move_and_resize(geom_midi_monitor_rect());
}

void GUI::free_windows() {
  delete set_lists;
  delete set_list;
  delete song;
  delete patch;
  delete message;
  delete triggers;
  delete messages;
  delete info;

  delete play_song;
  delete play_notes;
  delete play_patch;

  if (midi_monitor != nullptr)
    delete midi_monitor;
}

void GUI::toggle_view() {
  layout = layout == CURSES_LAYOUT_NORMAL ? CURSES_LAYOUT_PLAY : CURSES_LAYOUT_NORMAL;
}

void GUI::toggle_midi_monitor() {
  if (midi_monitor == nullptr) {
    midi_monitor = new MIDIMonitorWindow(geom_midi_monitor_rect(), km);
  }
  else {
    delete midi_monitor;
    midi_monitor = nullptr;
  }
}

void GUI::toggle_midi_monitor_show_clock() {
  if (midi_monitor != nullptr)
    midi_monitor->toggle_show_clock();
}

void GUI::refresh_all() {
  set_window_data();
  switch (layout) {
  case CURSES_LAYOUT_NORMAL:
    set_lists->draw();
    set_list->draw();
    song->draw();
    patch->draw();
    messages->draw();
    triggers->draw();
    message->draw();
    info->draw();
    break;
  case CURSES_LAYOUT_PLAY:
    play_song->draw();
    play_notes->draw();
    play_patch->draw();
    break;
  }
  if (midi_monitor != nullptr)
    midi_monitor->draw();

  wnoutrefresh(stdscr);

  switch (layout) {
  case CURSES_LAYOUT_NORMAL:
    wnoutrefresh(set_lists->win);
    wnoutrefresh(set_list->win);
    wnoutrefresh(song->win);
    wnoutrefresh(patch->win);
    wnoutrefresh(info->win);
    wnoutrefresh(messages->win);
    wnoutrefresh(triggers->win);
    break;
  case CURSES_LAYOUT_PLAY:
    wnoutrefresh(play_song->win);
    wnoutrefresh(play_notes->win);
    wnoutrefresh(play_patch->win);
    break;
  }

  if (midi_monitor != nullptr)
    wnoutrefresh(midi_monitor->win);

  doupdate();
}

void GUI::set_window_data() {
  switch (layout) {
  case CURSES_LAYOUT_NORMAL:
    set_normal_window_data();
    break;
  case CURSES_LAYOUT_PLAY:
    set_play_window_data();
    break;
  }
}

void GUI::set_normal_window_data() {
  SetList *sl = km->cursor()->set_list();
  Song *s = km->cursor()->song();
  Patch *p = km->cursor()->patch();

  set_lists->set_contents("Song Lists", &km->set_lists(), km->cursor()->set_list());
  set_list->set_contents(sl->name().c_str(), &sl->songs(), km->cursor()->song());
  messages->set_contents("Messages", &km->messages(), 0);
  triggers->set_contents("Triggers", &km->triggers(), 0);

  if (s != nullptr) {
    song->set_contents(s->name().c_str(), &s->patches(), km->cursor()->patch());
    info->set_contents(&s->notes());
    patch->set_contents(p);
  }
  else {
    song->set_contents(0, 0, 0);
    info->set_contents((string *)nullptr);
    patch->set_contents(nullptr);
  }
}

void GUI::set_play_window_data() {
  Song *s = km->cursor()->song();
  Patch *p = km->cursor()->patch();

  if (s != nullptr) {
    play_song->set_contents(s->name().c_str(), &s->patches(), km->cursor()->patch());
    play_notes->set_contents(&s->notes());
    play_patch->set_contents(p);
  }
  else {
    play_song->set_contents(nullptr, nullptr, nullptr);
    play_notes->set_contents((string *)nullptr);
    play_patch->set_contents(nullptr);
  }
}

void GUI::close_screen() {
  curs_set(1);
  echo();
  nl();
  noraw();
  nocbreak();
  refresh();
  endwin();
}

void GUI::load() {
  PromptWindow *pwin = new PromptWindow("Load File");
  string path = pwin->gets();
  delete pwin;

  load(path);
}

void GUI::load(string path) {
  if (path.length() == 0) {
    show_message("error: no file loaded");
    return;
  }

  bool testing = km->is_testing();
  KeyMaster *old_km = km;
  Storage storage(path.c_str());
  km = storage.load(testing);
  delete old_km;

  ostringstream ostr;
  if (storage.has_error()) {
    ostr << "error: " << storage.error();
    show_message(ostr.str());
    return;
  }

  km->start();

  ostr << "loaded file " << path;
  show_message(ostr.str());
  last_loaded_file_path = path;
}

void GUI::reload() {
  load(last_loaded_file_path);
}

void GUI::save() {
  PromptWindow *pwin = new PromptWindow("Save File");
  string path = pwin->gets();
  delete pwin;

  save(path);
}

void GUI::save(string path) {
  if (path.length() == 0) {
    show_message("error: no file saved");
    return;
  }

  Storage storage(path.c_str());
  storage.save(km, km->is_testing());

  ostringstream ostr;
  if (storage.has_error()) {
    ostr << "error: " << storage.error();
    show_message(ostr.str());
    return;
  }

  ostr << "saved file " << path;
  show_message(ostr.str());
  last_loaded_file_path = path;
}

void GUI::help() {
  rect r = geom_help_rect();
  HelpWindow hw(r, "Help");
  hw.draw();
  wnoutrefresh(hw.win);
  doupdate();
  getch();                      /* wait for key and eat it */
}

void GUI::show_message(string msg) {
  WINDOW *win = message->win;
  wclear(win);
  message->make_fit(msg, 0);
  waddstr(win, msg.c_str());
  wrefresh(win);
  doupdate();
}

void GUI::clear_message() {
  WINDOW *win = message->win;
  wclear(win);
  wrefresh(win);
  doupdate();
}

void *clear_message_thread(void *gui_vptr) {
  GUI *gui = (GUI *)gui_vptr;
  int clear_message_id = gui->clear_message_id();

  sleep(gui->clear_message_seconds());

  // Only clear the window if the id hasn't changed
  if (gui->clear_message_id() == clear_message_id)
    gui->clear_message();
  return nullptr;
}

void GUI::clear_message_after(int secs) {
  clear_msg_secs = secs;
  clear_msg_id++;

  pthread_t pthread;
  pthread_create(&pthread, 0, clear_message_thread, this);
}
