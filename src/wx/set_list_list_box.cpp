#include "set_list_list_box.h"
#include "../keymaster.h"
#include "../cursor.h"

SetListListBox::SetListListBox(wxWindow *parent, wxWindowID id, wxSize size)
  : FrameListBox(parent, id, wxDefaultPosition, size, wxLB_SINGLE)
{
}

void SetListListBox::update() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor;

  Clear();
  wxArrayString names;
  for (auto& set_list : km->set_lists)
    names.Add(set_list->name.c_str());
  if (!names.empty())
    InsertItems(names, 0);

  int i = 0;
  for (auto& set_list : km->set_lists) {
    if (set_list == cursor->set_list()) {
      SetSelection(i);
      break;;
    }
    ++i;
  }

  FrameListBox::update();
}
