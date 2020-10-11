#ifndef SET_LIST_EDITOR_H
#define SET_LIST_EDITOR_H

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "events.h"

enum {
  ID_SLE_Name = 1000,
  ID_SLE_AllSongs,
  ID_SLE_SetList,
  ID_SLE_AddButton,
  ID_SLE_RemoveButton,
  ID_SLE_MoveUp,
  ID_SLE_MoveDown
};

class Song;
class KeyMaster;
class SetList;
class wxListCtrl;

class SetListEditor : public wxDialog {
public:
  SetListEditor(wxWindow *parent, SetList *set_list);

private:
  KeyMaster *km;
  SetList *set_list;
  std::vector<Song *>songs_copy;
  wxTextCtrl *name_text;
  wxListBox *all_songs_wxlist;
  wxListBox *set_list_wxlist;
  wxButton *add_button;
  wxButton *remove_button;
  wxButton *up_button;
  wxButton *down_button;

  wxWindow *make_name_panel(wxWindow *parent);
  wxWindow *make_all_songs_panel(wxWindow *parent);
  wxWindow *make_buttons(wxWindow *parent);
  wxWindow *make_set_list_panel(wxWindow *parent);
  wxWindow *make_panel(wxWindow *parent, wxWindowID id,
                       const char * const title,
                       std::vector<Song *> &slist,
                       wxListBox **list_ptr);

  void set_name(wxCommandEvent& event);
  void all_songs_selection(wxCommandEvent& event);
  void set_list_selection(wxCommandEvent& event);
  void add_song(wxCommandEvent& event);
  void remove_song(wxCommandEvent& event);
  void move_song_up(wxCommandEvent& event);
  void move_song_down(wxCommandEvent& event);

  void update(wxListBox *list_box, std::vector<Song *> &song_list);
  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* SET_LIST_EDITOR_H */
