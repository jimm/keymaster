#include "set_list_box.h"
#include "../keymaster.h"
#include "../cursor.h"

SetListBox::SetListBox(wxWindow *parent, wxWindowID id, wxSize size)
  : FrameListBox(parent, id, wxDefaultPosition, size, wxLB_SINGLE),
    set_list(nullptr)
{
}

void SetListBox::update() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor;
  SetList *curr_set_list = cursor->set_list();

  Clear();
  if (curr_set_list != nullptr) {
    wxArrayString names;
    for (auto& song : curr_set_list->songs)
      names.Add(song->name.c_str());
    if (!names.IsEmpty())
      InsertItems(names, 0);
  }
  set_list = curr_set_list;

  if (set_list == nullptr)
    return;

  int i = 0;
  for (auto& song : set_list->songs) {
    if (song == cursor->song()) {
      SetSelection(i);
      break;
    }
    ++i;
  }

  FrameListBox::update();
}

void SetListBox::jump() {
  int selection = GetSelection();
  if (selection != wxNOT_FOUND) {
    KeyMaster *km = KeyMaster_instance();
    km->jump_to_song_index(selection);
  }
}
