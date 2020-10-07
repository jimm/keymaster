#include <wx/defs.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/gbsizer.h>
#include <unistd.h>
#include "frame.h"
#include "macros.h"
#include "set_list_box.h"
#include "set_list_list_box.h"
#include "song_box.h"
#include "patch_connections.h"
#include "message_list.h"
#include "trigger_list.h"
#include "instrument_dialog.h"
#include "monitor.h"
#include "message_editor.h"
#include "trigger_editor.h"
#include "connection_editor.h"
#include "set_list_editor.h"
#include "song_editor.h"
#include "patch_editor.h"
#include "clock_panel.h"
#include "../keymaster.h"
#include "../cursor.h"
#include "../storage.h"
#include "../editor.h"

#define LIST_WIDTH 200
#define TALL_LIST_HEIGHT 300
#define SHORT_LIST_HEIGHT 200
#define CLOCK_BPM_HEIGHT 16
#define NOTES_WIDTH 200

wxDEFINE_EVENT(Frame_MenuUpdate, wxCommandEvent);

wxBEGIN_EVENT_TABLE(Frame, wxFrame)
  EVT_MENU(wxID_NEW,  Frame::OnNew)
  EVT_MENU(wxID_OPEN,  Frame::OnOpen)
  EVT_MENU(wxID_SAVE,  Frame::OnSave)
  EVT_MENU(wxID_SAVEAS,  Frame::OnSaveAs)
  EVT_MENU(ID_GoNextSong, Frame::next_song)
  EVT_MENU(ID_GoPrevSong, Frame::prev_song)
  EVT_MENU(ID_GoNextPatch, Frame::next_patch)
  EVT_MENU(ID_GoPrevPatch, Frame::prev_patch)
  EVT_MENU(ID_FindSetList, Frame::find_set_list)
  EVT_MENU(ID_FindSong, Frame::find_song)
  EVT_MENU(ID_CreateMessage, Frame::create_message)
  EVT_MENU(ID_CreateTrigger, Frame::create_trigger)
  EVT_MENU(ID_CreateSong, Frame::create_song)
  EVT_MENU(ID_CreatePatch, Frame::create_patch)
  EVT_MENU(ID_CreateConnection, Frame::create_connection)
  EVT_MENU(ID_CreateSetList, Frame::create_set_list)
  EVT_MENU(ID_DestroyMessage, Frame::destroy_message)
  EVT_MENU(ID_DestroyTrigger, Frame::destroy_trigger)
  EVT_MENU(ID_DestroySong, Frame::destroy_song)
  EVT_MENU(ID_DestroyPatch, Frame::destroy_patch)
  EVT_MENU(ID_DestroyConnection, Frame::destroy_connection)
  EVT_MENU(ID_DestroySetList, Frame::destroy_set_list)
  EVT_MENU(ID_ListInstruments, Frame::OnListInstruments)
  EVT_MENU(ID_Monitor, Frame::OnMonitor)
  EVT_MENU(ID_ClockToggle, Frame::toggle_clock)
  EVT_MENU(ID_RegularPanic, Frame::regular_panic)
  EVT_MENU(ID_SuperPanic, Frame::super_panic)
  EVT_MENU(wxID_ABOUT, Frame::OnAbout)

  EVT_LISTBOX(ID_SetListList, Frame::jump_to_set_list)
  EVT_LISTBOX_DCLICK(ID_SetListList, Frame::edit_set_list)
  EVT_LISTBOX(ID_SetListSongs, Frame::jump_to_song)
  EVT_LISTBOX_DCLICK(ID_SetListSongs, Frame::edit_song)
  EVT_LISTBOX(ID_SongPatches, Frame::jump_to_patch)
  EVT_LISTBOX_DCLICK(ID_SongPatches, Frame::edit_patch)
  EVT_LISTBOX(ID_MessageList, Frame::send_message)
  EVT_LISTBOX_DCLICK(ID_MessageList, Frame::edit_message)

  EVT_LIST_ITEM_ACTIVATED(ID_TriggerList, Frame::edit_trigger)
  EVT_LIST_ITEM_ACTIVATED(ID_PatchConnections, Frame::edit_connection)

  EVT_TEXT(ID_SongNotes, Frame::set_song_notes)

  EVT_COMMAND(wxID_ANY, Frame_MenuUpdate, Frame::update_menu_items)
wxEND_EVENT_TABLE()

void *frame_clear_user_message_thread(void *gui_vptr) {
  Frame *gui = (Frame *)gui_vptr;
  int clear_user_message_id = gui->clear_user_message_id();

  sleep(gui->clear_user_message_seconds());

  // Only clear the window if the id hasn't changed
  if (gui->clear_user_message_id() == clear_user_message_id)
    gui->clear_user_message();
  return nullptr;
}

Frame::Frame(const wxString& title)
  : wxFrame(NULL, wxID_ANY, title),
    updating_notes(false)
{
  make_frame_panels();
  make_menu_bar();
  CreateStatusBar();
  show_user_message("No KeyMaster file loaded", 15);
}

void Frame::make_frame_panels() {
  wxGridBagSizer * const sizer = new wxGridBagSizer();

  sizer->Add(make_set_list_songs_panel(this), POS(0, 0), SPAN(3, 1), wxEXPAND);
  sizer->Add(make_song_patches_panel(this), POS(0, 1), SPAN(3, 1), wxEXPAND);
  sizer->Add(make_clock_panel(this), POS(0, 2), SPAN(1, 1), wxEXPAND);
  sizer->Add(make_notes_panel(this), POS(1, 2), SPAN(2, 1), wxEXPAND);

  sizer->Add(make_patch_conns_panel(this), POS(3, 0), SPAN(1, 3), wxEXPAND);

  sizer->Add(make_set_lists_panel(this), POS(4, 0), SPAN(1, 1), wxEXPAND);
  sizer->Add(make_messages_panel(this), POS(4, 1), SPAN(1, 1), wxEXPAND);
  sizer->Add(make_triggers_panel(this), POS(4, 2), SPAN(1, 1), wxEXPAND);


  wxBoxSizer * const outer_border_sizer = new wxBoxSizer(wxVERTICAL);
  outer_border_sizer->Add(sizer, wxSizerFlags().Expand().Border());
  SetSizerAndFit(outer_border_sizer);
}

wxWindow * Frame::make_set_list_songs_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_set_list = new SetListBox(p, ID_SetListSongs,
                               wxSize(LIST_WIDTH, TALL_LIST_HEIGHT));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, "Songs"), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_set_list, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_set_lists_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_set_lists = new SetListListBox(p, ID_SetListList,
                                      wxSize(LIST_WIDTH, SHORT_LIST_HEIGHT));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Set Lists")), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_set_lists, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_song_patches_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_song_patches = new SongBox(p, ID_SongPatches,
                                wxSize(LIST_WIDTH, TALL_LIST_HEIGHT));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Patches")), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_song_patches, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_messages_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_messages = new MessageList(p, ID_MessageList,
                                wxSize(LIST_WIDTH, SHORT_LIST_HEIGHT));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, "Messages"), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_messages, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_triggers_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_triggers = new TriggerList(p, ID_TriggerList);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, "Triggers"), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_triggers, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_clock_panel(wxWindow *parent) {
  return clock_panel = new ClockPanel(parent);
}

wxWindow * Frame::make_notes_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  lc_notes = new wxTextCtrl(p, ID_SongNotes, "", wxDefaultPosition,
                            wxSize(NOTES_WIDTH, TALL_LIST_HEIGHT), wxTE_MULTILINE);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(p, wxID_ANY, "Notes"), wxSizerFlags().Align(wxALIGN_LEFT));
  sizer->Add(lc_notes, wxSizerFlags(1).Expand().Border());

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow * Frame::make_patch_conns_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  lc_patch_conns = new PatchConnections(p, ID_PatchConnections);
  sizer->Add(lc_patch_conns, wxSizerFlags(1).Expand().Border());
  p->SetSizerAndFit(sizer);
  return p;
}

void Frame::make_menu_bar() {
  wxMenu *menu_file = new wxMenu;
  menu_file->Append(wxID_NEW, "&New Project\tCtrl-Shift-N", "Create a new project");
  menu_file->Append(wxID_OPEN);
  menu_file->Append(wxID_SAVE);
  menu_file->Append(wxID_SAVEAS);
  menu_file->AppendSeparator();
  menu_file->Append(wxID_EXIT);

  wxMenu *menu_edit = new wxMenu;
  menu_edit->Append(ID_CreateMessage, "New Message\tCtrl-Shift-M", "Create a new message");
  menu_edit->Append(ID_CreateTrigger, "New Trigger\tCtrl-Shift-T", "Create a new trigger");
  menu_edit->Append(ID_CreateSong, "New Song\tCtrl-Shift-S", "Create a new song");
  menu_edit->Append(ID_CreatePatch, "New Patch\tCtrl-Shift-P", "Create a new patch");
  menu_edit->Append(ID_CreateConnection, "New Connection\tCtrl-Shift-C", "Create a new connection");
  menu_edit->Append(ID_CreateSetList, "New Set List\tCtrl-Shift-L", "Create a new set list");
  menu_edit->AppendSeparator();
  menu_edit->Append(ID_DestroyMessage, "Delete Message\tCtrl-Alt-M", "Delete the current message");
  menu_edit->Append(ID_DestroyTrigger, "Delete Trigger\tCtrl-Alt-T", "Delete the current trigger");
  menu_edit->Append(ID_DestroySong, "Delete Song\tCtrl-Alt-S", "Delete the current song");
  menu_edit->Append(ID_DestroyPatch, "Delete Patch\tCtrl-Alt-P", "Delete the current patch");
  menu_edit->Append(ID_DestroyConnection, "Delete Connection\tCtrl-Alt-C", "Delete the current connection");
  menu_edit->Append(ID_DestroySetList, "Delete Set List\tCtrl-Alt-L", "Delete the current set list");

  wxMenu *menu_go = new wxMenu;
  menu_go->Append(ID_GoNextSong, "Next Song\tCtrl-Right", "Move to the next song");
  menu_go->Append(ID_GoPrevSong, "Prev Song\tCtrl-Left", "Move to the previous song");
  menu_go->Append(ID_GoNextPatch, "Next Patch\tCtrl-Down", "Move to the next patch");
  menu_go->Append(ID_GoPrevPatch, "Prev Patch\tCtrl-Up", "Move to the previous patch");
  menu_go->AppendSeparator();
  menu_go->Append(ID_FindSong, "Find Song...\tCtrl-F", "Find song by name");
  menu_go->Append(ID_FindSetList, "Find Set List...\tCtrl-T", "Find set list by name");

  wxMenu *menu_windows = new wxMenu;
  menu_windows->Append(ID_ListInstruments, "&Instruments\tCtrl-I",
                      "Displays input and output instruments");
  menu_windows->Append(ID_Monitor, "MIDI &Monitor\tCtrl-M",
                      "Open the MIDI Monitor window");

  wxMenu *menu_midi = new wxMenu;
  menu_midi->Append(ID_ClockToggle, "Start/Stop MIDI Clock\tCtrl-K", "Start or stop the MIDI clock");
  menu_midi->AppendSeparator();
  menu_midi->Append(ID_RegularPanic, "Send &All Notes Off\tCtrl-.",
                   "Send All Notes Off controller message on all channels");
  menu_midi->Append(ID_SuperPanic, "Send Super-&Panic\tCtrl-/",
                   "Send Notes Off messages, all notes, all channels");

  wxMenu *menu_help = new wxMenu;
  menu_help->Append(wxID_ABOUT);

  menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, "&File");
  menu_bar->Append(menu_edit, "&Edit");
  menu_bar->Append(menu_go, "&Go");
  menu_bar->Append(menu_midi, "&MIDI");
  menu_bar->Append(menu_windows, "&Windows");
  menu_bar->Append(menu_help, "&Help");
  SetMenuBar(menu_bar);
#if defined(__WXMAC__)
  menu_bar->OSXGetAppleMenu()->SetTitle("KeyMaster");
#endif
}

// ================ messaging ================

void Frame::show_user_message(string msg) {
  SetStatusText(msg.c_str());
}

void Frame::show_user_message(string msg, int secs) {
  SetStatusText(msg.c_str());
  clear_user_message_after(secs);
}

void Frame::clear_user_message() {
  SetStatusText("");
}

void Frame::clear_user_message_after(int secs) {
  clear_msg_secs = secs;
  clear_msg_id++;

  pthread_t pthread;
  pthread_create(&pthread, 0, frame_clear_user_message_thread, this);
}

// ================ movement ================

void Frame::next_song() {
  KeyMaster *km = KeyMaster_instance();
  km->next_song();
  update();
}

void Frame::prev_song() {
  KeyMaster *km = KeyMaster_instance();
  km->prev_song();
  update();
}

void Frame::next_patch() {
  KeyMaster *km = KeyMaster_instance();
  km->next_patch();
  update();
}

void Frame::prev_patch() {
  KeyMaster *km = KeyMaster_instance();
  km->prev_patch();
  update();
}

void Frame::find_set_list() {
  KeyMaster *km = KeyMaster_instance();
  wxTextEntryDialog prompt(this, "Find Set List", "Find Set List");
  if (prompt.ShowModal() == wxID_OK) {
    wxString str = prompt.GetValue();
    if (!str.IsEmpty()) {
      km->goto_set_list(str.ToStdString());
      update();
    }
  }
}

void Frame::find_song() {
  KeyMaster *km = KeyMaster_instance();
  wxTextEntryDialog prompt(this, "Find Song", "Find Song");
  if (prompt.ShowModal() == wxID_OK) {
    wxString str = prompt.GetValue();
    if (!str.IsEmpty()) {
      km->goto_song(str.ToStdString());
      update();
    }
  }
}

void Frame::jump_to_set_list(wxCommandEvent &event) {
  if (event.GetEventType() == wxEVT_LISTBOX && event.IsSelection()) {
    lc_set_lists->jump();
    update();
  }
}

void Frame::jump_to_song(wxCommandEvent &event) {
  if (event.GetEventType() == wxEVT_LISTBOX && event.IsSelection()) {
    lc_set_list->jump();
    update();
  }
}

void Frame::jump_to_patch(wxCommandEvent &event) {
  if (event.GetEventType() == wxEVT_LISTBOX && event.IsSelection()) {
    lc_song_patches->jump();
    update();
  }
}

// ================ messages ================

void Frame::send_message(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  int message_num = lc_messages->GetSelection();
  Message *message = km->messages[message_num];
  message->send_to_all_outputs();
}

// ================ create, edit, destroy ================

void Frame::create_message(wxCommandEvent& event) {
  Editor e;
  Message *message = e.create_message();
  if (edit_message(message)) {
    e.add_message(message);
    update();
  }
  else
    delete message;
}

void Frame::create_trigger(wxCommandEvent& event) {
  Editor e;
  KeyMaster *km = KeyMaster_instance();
  Trigger *trigger = e.create_trigger(km->inputs.front());
  if (edit_trigger(trigger)) {
    e.add_trigger(trigger);
    update();
  }
  else
    delete trigger;
}

void Frame::create_song(wxCommandEvent& event) {
  Editor e;
  Song *song = e.create_song();
  if (edit_song(song)) {
    e.add_song(song);
    update();
  }
  else
    delete song;
}

void Frame::create_patch(wxCommandEvent& event) {
  Editor e;
  Patch *patch = e.create_patch();
  if (edit_patch(patch)) {
    e.add_patch(patch);
    update();
  }
  else
    delete patch;
}

void Frame::create_connection(wxCommandEvent& event) {
  Editor e;
  KeyMaster *km = KeyMaster_instance();
  if (km->inputs.empty() || km->outputs.empty()) {
    wxMessageBox("There must be at least one input and one\noutput to create a connection",
                "New Connection", wxOK | wxICON_INFORMATION);
    return;
  }
  Patch *patch = km->cursor->patch();
  if (patch == nullptr) {
    wxMessageBox("Please select a patch", "New Connection",
                 wxOK | wxICON_INFORMATION);
    return;
  }

  Connection *conn = e.create_connection(km->inputs.front(), km->outputs.front());
  if (edit_connection(conn)) {
    e.add_connection(conn, patch);
    update();
  }
  else
    delete conn;
}

void Frame::create_set_list(wxCommandEvent& event) {
  Editor e;
  SetList *set_list = e.create_set_list();
  if (edit_set_list(set_list)) {
    e.add_set_list(set_list);
    update();
  }
  else
    delete set_list;
}

void Frame::edit_message(wxCommandEvent& event) {
  int message_num = lc_messages->GetSelection();
  if (message_num != wxNOT_FOUND)
    edit_message(KeyMaster_instance()->messages[message_num]);
}

bool Frame::edit_message(Message *message) {
  if (message != nullptr)
    if (MessageEditor(this, message).ShowModal() == wxID_OK) {
      update();
      return true;
    }
  return false;
}

void Frame::edit_trigger(wxListEvent& event) {
  edit_trigger(lc_triggers->selected());
}

bool Frame::edit_trigger(Trigger *trigger) {
  if (trigger != nullptr)
    if (TriggerEditor(this, trigger).ShowModal() == wxID_OK) {
      update();
      return true;
    }
  return false;
}

void Frame::edit_set_list(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  edit_set_list(km->cursor->set_list());
}

bool Frame::edit_set_list(SetList *set_list) {
  if (set_list == nullptr)
    return false;

  KeyMaster *km = KeyMaster_instance();
  if (set_list == km->all_songs) {
    wxMessageBox("Can't edit the main list of all songs",
                "Set List Editor", wxOK | wxICON_INFORMATION);
    return false;
  }
  if (SetListEditor(this, set_list).ShowModal() == wxID_OK) {
    update();
    return true;
  }
  return false;
}

void Frame::edit_song(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  edit_song(km->cursor->song());
}

bool Frame::edit_song(Song *song) {
  if (song == nullptr)
    return false;

  if (SongEditor(this, song).ShowModal() == wxID_OK) {
    KeyMaster *km = KeyMaster_instance();
    km->sort_all_songs();
    km->update_clock();
    update();
    return true;
  }
  return false;
}

void Frame::edit_patch(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  edit_patch(km->cursor->patch());
}

bool Frame::edit_patch(Patch *patch) {
  if (patch != nullptr)
    if (PatchEditor(this, patch).ShowModal() == wxID_OK) {
      update();
      return true;
    }
  return false;
}

void Frame::edit_connection(wxListEvent& event) {
  edit_connection(lc_patch_conns->selected());
}

bool Frame::edit_connection(Connection *conn) {
  if (conn == nullptr)
    return false;

  if (ConnectionEditor(this, conn).ShowModal() == wxID_OK) {
    update();
    return true;
  }
  return false;
}

void Frame::set_song_notes(wxCommandEvent& event) {
  if (updating_notes)
    return;
  Song *song = KeyMaster_instance()->cursor->song();
  if (song != nullptr)
    song->notes = lc_notes->GetValue();
}

void Frame::destroy_message(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km->messages.empty())
    return;

  Editor e;
  int message_num = lc_messages->GetSelection();
  if (message_num == wxNOT_FOUND)
    return;
  Message *message = km->messages[message_num];
  e.destroy_message(message);
  update();
}

void Frame::destroy_trigger(wxCommandEvent& event) {
  Editor e;
  e.destroy_trigger(lc_triggers->selected());
}

void Frame::destroy_song(wxCommandEvent& event) {
  Editor e;
  KeyMaster *km = KeyMaster_instance();
  Song *song = km->cursor->song();
  if (song != nullptr)
    e.destroy_song(song);
  update();
}

void Frame::destroy_patch(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  Patch *patch = km->cursor->patch();
  if (patch == nullptr)
    return;

  Editor e;
  e.destroy_patch(km->cursor->song(), patch);
  update();
}

void Frame::destroy_connection(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  Patch *patch = km->cursor->patch();
  if (patch == nullptr)
    return;

  Connection *conn = lc_patch_conns->selected();
  if (conn == nullptr)
    return;

  Editor e;
  e.destroy_connection(patch, conn);
  update();
}

void Frame::destroy_set_list(wxCommandEvent& event) {
  Editor e;
  KeyMaster *km = KeyMaster_instance();
  SetList *set_list = km->cursor->set_list();
  if (set_list != nullptr && set_list != km->all_songs)
    e.destroy_set_list(set_list);
  update();
}

// ================ keypress handling ================

int Frame::handle_global_key_event(wxKeyEvent &event) {
  int key_code = event.GetKeyCode();

  if (key_code >= WXK_F1 && key_code <= WXK_F24) {
    if (handle_trigger_key(key_code)) {
      update();
      return true;
    }
  }

  // Do not handle keys if we're in a text control.
  wxString focus_class_name = FindFocus()->GetClassInfo()->GetClassName();
  if (strncmp("wxTextCtrl", focus_class_name.c_str(), 10) == 0)
    return -1;

  switch (event.GetKeyCode()) {
  case WXK_LEFT: case 'K':
    prev_song();
    break;
  case WXK_RIGHT: case 'J':
    next_song();
    break;
  case WXK_UP: case 'P':
    prev_patch();
    break;
  case WXK_DOWN: case 'N':
    next_patch();
    break;
  case ' ':
    if (event.ShiftDown())
      prev_patch();
    else
      next_patch();
    break;
  default:
    return -1;
  }

  update();
  return true;
}

// ================ MIDI menu actions ================

void Frame::toggle_clock(wxCommandEvent &_event) {
  KeyMaster_instance()->toggle_clock();
  // clock will send "changed" message which we observe which will tell us
  // to update the clock panel.
}

void Frame::regular_panic(wxCommandEvent &_event) {
  KeyMaster *km = KeyMaster_instance();
  show_user_message("Sending panic...");
  km->panic(false);
  show_user_message("Panic sent", 5);
}

void Frame::super_panic(wxCommandEvent &_event) {
  KeyMaster *km = KeyMaster_instance();
  show_user_message("Sending \"super panic\": all notes off, all channels...");
  km->panic(true);
  show_user_message("Super panic sent (all notes off, all channels)", 5);
}

// ================ trigger keys

bool Frame::handle_trigger_key(int key_code) {
  bool triggered = false;

  for (auto &trigger : KeyMaster_instance()->triggers)
    if (trigger->signal_key(key_code))
      triggered = true;

  return triggered;
}

// ================ standard menu items ================

void Frame::OnAbout(wxCommandEvent &_event) {
  wxMessageBox("KeyMaster, the MIDI processing and patching system.\n"
               "v1.0.0\n"
               "Jim Menard, jim@jimmenard.com\n"
               "https://github.com/jimm/keymaster/wiki",
               "About KeyMaster", wxOK | wxICON_INFORMATION);
}

void Frame::OnNew(wxCommandEvent &_event) {
  create_new_keymaster();
}

void Frame::OnOpen(wxCommandEvent &_event) {
  wxFileName fname(file_path);

  wxFileDialog openFileDialog(this, "Open KeyMaster file",
                              fname.GetPath(), fname.GetFullName(),
                              "KeyMaster files (*.km)|*.km",
                              wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() != wxID_CANCEL) {
    file_path = openFileDialog.GetPath();
    load(file_path);
  }
  update_menu_items();
}

void Frame::OnSave(wxCommandEvent &event) {
  if (file_path.empty()) {
    OnSaveAs(event);
    return;
  }
  save();
  update_menu_items();
}

void Frame::OnSaveAs(wxCommandEvent &_event) {
  wxFileName fname(file_path);

  wxFileDialog file_dialog(this, "Save KeyMaster file",
                           fname.GetPath(), fname.GetFullName(),
                           "KeyMaster files (*.km)|*.km",
                           wxFD_SAVE);
  if (file_dialog.ShowModal() != wxID_CANCEL) {
    file_path = file_dialog.GetPath();
    save();
    update_menu_items();
  }
}

// ================ windows ================

void Frame::OnListInstruments(wxCommandEvent &_event) {
  InstrumentDialog(this, KeyMaster_instance()).run();
}

void Frame::OnMonitor(wxCommandEvent &event) {
  new Monitor();
}

// ================ helpers ================

void Frame::initialize() {
  KeyMaster *km = new KeyMaster();
  km->initialize();
  km->start();
  km->clock.add_observer(this);
  update();
}

void Frame::load(wxString path) {
  if (access(path, F_OK) != 0) {
    wxString err = wxString::Format("File '%s' does not exist", path);
    wxLogError(err);
    if (KeyMaster_instance() == nullptr)
      create_new_keymaster();
    return;
  }

  KeyMaster *old_km = KeyMaster_instance();
  if (old_km != nullptr)
    old_km->clock.remove_observer(this);
  bool testing = old_km != nullptr && old_km->testing;

  Storage storage(path);
  KeyMaster *km = storage.load(testing);
  if (storage.has_error()) {
    wxLogError("Cannot open file '%s': %s.", path, storage.error());
    return;
  }

  show_user_message(string(wxString::Format("Loaded %s", path).c_str()), 15);
  file_path = path;

  if (old_km != nullptr) {
    old_km->stop();
    delete old_km;
  }
  km->start();                  // initializes cursor
  km->clock.add_observer(this);
  update();                     // must come after start
}

void Frame::create_new_keymaster() {
  KeyMaster *old_km = KeyMaster_instance();
  bool testing = old_km != nullptr && old_km->testing;

  KeyMaster *km = new KeyMaster();
  km->testing = testing;
  km->initialize();

  if (old_km != nullptr) {
    old_km->stop();
    old_km->clock.remove_observer(this);
    delete old_km;
  }

  show_user_message("Created new project", 15);
  file_path = "";
  km->start();                  // initializes cursor
  km->clock.add_observer(this);
  update();                     // must come after start
}

void Frame::save() {
  if (file_path.empty())
    return;

  Storage storage(file_path);
  storage.save(KeyMaster_instance());
}

void Frame::update(Observable *o, void *arg) {
  // Right now we only observe the clock
  ClockChange clock_update = (ClockChange)(long)arg;
  switch (clock_update) {
  case ClockChangeBpm:
  case ClockChangeStart:
  case ClockChangeStop:
    clock_panel->update();
    break;
  default:                      // ignore beats, for example
    break;
  }
}

void Frame::update() {
  clock_panel->update();
  update_lists();
  update_song_notes();
  update_menu_items();
}

void Frame::update_lists() {
  lc_set_list->update();
  lc_set_lists->update();
  lc_song_patches->update();
  lc_patch_conns->update();
  lc_messages->update();
  lc_triggers->update();
}

void Frame::update_song_notes() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor;

  Song *song = cursor->song();
  updating_notes = true;
  lc_notes->Clear();
  if (song != nullptr)
    lc_notes->AppendText(song->notes);
  updating_notes = false;
}

void Frame::update_menu_items() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor;
  Editor e(km);

  // file menu
  menu_bar->FindItem(wxID_SAVEAS, nullptr)->Enable(!file_path.empty());

  // edit menu
  menu_bar->FindItem(ID_DestroyMessage, nullptr)
    ->Enable(!km->messages.empty() && lc_messages->GetSelection() != wxNOT_FOUND);

  menu_bar->FindItem(ID_DestroyTrigger, nullptr)
    ->Enable(lc_triggers->selected() != nullptr);

  menu_bar->FindItem(ID_DestroySong, nullptr)
    ->Enable(cursor->song() != nullptr);

  bool enable = cursor->patch() != nullptr &&
    cursor->song()->patches.size() > 1;
  menu_bar->FindItem(ID_DestroyPatch, nullptr)->Enable(enable);

  menu_bar->FindItem(ID_DestroyConnection, nullptr)
    ->Enable(e.ok_to_destroy_connection(
               cursor->patch(), lc_patch_conns->selected()));

  menu_bar->FindItem(ID_DestroySetList, nullptr)
             ->Enable(e.ok_to_destroy_set_list(cursor->set_list()));

  // go menu
  menu_bar->FindItem(ID_GoNextSong, nullptr)->Enable(cursor->has_next_song());
  menu_bar->FindItem(ID_GoPrevSong, nullptr)->Enable(cursor->has_prev_song());
  menu_bar->FindItem(ID_GoNextPatch, nullptr)->Enable(cursor->has_next_patch());
  menu_bar->FindItem(ID_GoPrevPatch, nullptr)->Enable(cursor->has_prev_patch());

  SetList *set_list = cursor->set_list();
  bool has_songs = set_list != nullptr && !set_list->songs.empty();
  menu_bar->FindItem(ID_FindSong, nullptr)->Enable(has_songs);

  menu_bar->FindItem(ID_FindSetList, nullptr)->Enable(km->set_lists.size() > 1);
}
