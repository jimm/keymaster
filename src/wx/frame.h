#ifndef WX_FRAME_H
#define WX_FRAME_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
 #include <wx/listctrl.h>
#endif
#include "events.h"
#include "../keymaster.h"
#include "../observer.h"

enum {
  ID_GoNextSong = 1,
  ID_GoPrevSong,
  ID_GoNextPatch,
  ID_GoPrevPatch,
  ID_FindSong,
  ID_FindSetList,
  ID_SetListList,
  ID_SetListSongs,
  ID_SongPatches,
  ID_PatchConnections,
  ID_CreateMessage,
  ID_CreateTrigger,
  ID_CreateSong,
  ID_CreatePatch,
  ID_CreateConnection,
  ID_CreateSetList,
  ID_DestroyMessage,
  ID_DestroyTrigger,
  ID_DestroySong,
  ID_DestroyPatch,
  ID_DestroyConnection,
  ID_DestroySetList,
  ID_ListInstruments,
  ID_Monitor,
  ID_ClockToggle,
  ID_RegularPanic,
  ID_SuperPanic,
  ID_MessageList,
  ID_TriggerList,
  ID_SongNotes
};

class wxTextCtrl;
class SetListBox;
class SetListListBox;
class SongBox;
class PatchConnections;
class MessageList;
class TriggerList;
class SetListEditor;
class ClockPanel;

class Frame: public wxFrame, public Observer {
public:
  Frame(const wxString& title);
  virtual ~Frame() {}

  void initialize();
  void load(wxString path);
  void save();

  int handle_global_key_event(wxKeyEvent &event);


  virtual void update(Observable *o, void *arg);
  void update(wxCommandEvent& event) { update(); }
  void update();

  void update_menu_items(wxCommandEvent& event) { update_menu_items(); }
  void update_menu_items();

  void show_user_message(std::string);
  void show_user_message(std::string, int);
  void clear_user_message();
  void clear_user_message_after(int);
  int clear_user_message_seconds() { return clear_msg_secs; }
  int clear_user_message_id() { return clear_msg_id; }

  void next_song();
  void prev_song();
  void next_patch();
  void prev_patch();
  void find_set_list();
  void find_song();

  void jump_to_set_list();
  void jump_to_song();
  void jump_to_patch();

private:
  wxString file_path;
  wxMenuBar *menu_bar;
  SetListListBox *lc_set_lists;
  SetListBox *lc_set_list;
  SongBox *lc_song_patches;
  PatchConnections *lc_patch_conns;
  ClockPanel *clock_panel;
  MessageList *lc_messages;
  TriggerList *lc_triggers;
  wxTextCtrl *lc_notes;
  bool updating_notes;
  int clear_msg_secs;
  int clear_msg_id;

  void OnNew(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnSaveAs(wxCommandEvent& event);
  void OnListInstruments(wxCommandEvent& event);
  void OnMonitor(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);

  void create_new_keymaster();

  void toggle_clock(wxCommandEvent &_event);
  void regular_panic(wxCommandEvent &_event);
  void super_panic(wxCommandEvent &_event);
  bool handle_trigger_key(int key_code);

  void next_song(wxCommandEvent &_event) { next_song(); }
  void prev_song(wxCommandEvent &_event) { prev_song(); }
  void next_patch(wxCommandEvent &_event) { next_patch(); }
  void prev_patch(wxCommandEvent &_event) { prev_patch(); }
  void find_set_list(wxCommandEvent &_event) { find_set_list(); }
  void find_song(wxCommandEvent &_event) { find_song(); }

  void jump_to_set_list(wxCommandEvent& event);
  void jump_to_song(wxCommandEvent& event);
  void jump_to_patch(wxCommandEvent& event);

  void create_message(wxCommandEvent& event);
  void create_trigger(wxCommandEvent& event);
  void create_song(wxCommandEvent& event);
  void create_patch(wxCommandEvent& event);
  void create_connection(wxCommandEvent& event);
  void create_set_list(wxCommandEvent& event);

  void send_message(wxCommandEvent& event);

  void edit_message(wxCommandEvent& event);
  bool edit_message(Message *);
  void edit_trigger(wxListEvent& event);
  bool edit_trigger(Trigger *);
  void edit_set_list(wxCommandEvent& event);
  bool edit_set_list(SetList *set_list);
  void edit_song(wxCommandEvent& event);
  bool edit_song(Song *);
  void edit_patch(wxCommandEvent& event);
  bool edit_patch(Patch *);
  void edit_connection(wxListEvent& event);
  bool edit_connection(Connection *);

  void set_song_notes(wxCommandEvent& event);

  void destroy_message(wxCommandEvent& event);
  void destroy_trigger(wxCommandEvent& event);
  void destroy_song(wxCommandEvent& event);
  void destroy_patch(wxCommandEvent& event);
  void destroy_connection(wxCommandEvent& event);
  void destroy_set_list(wxCommandEvent& event);

  void make_frame_panels();
  void make_menu_bar();
  wxWindow * make_set_list_songs_panel(wxWindow *);
  wxWindow * make_set_lists_panel(wxWindow *);
  wxWindow * make_song_patches_panel(wxWindow *);
  wxWindow * make_messages_panel(wxWindow *);
  wxWindow * make_triggers_panel(wxWindow *);
  wxWindow * make_clock_panel(wxWindow *);
  wxWindow * make_notes_panel(wxWindow *);
  wxWindow * make_patch_conns_panel(wxWindow *);

  bool dialog_closed(int dialog_return_code);
  void update_title();
  void update_lists();
  void update_song_notes();

  wxDECLARE_EVENT_TABLE();
};

#endif /* WX_FRAME_H */
