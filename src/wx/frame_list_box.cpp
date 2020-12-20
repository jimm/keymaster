#include "frame_list_box.h"
#include "events.h"

/*
 * A FrameListBox is a wxListCtrl that tells the frame to update its menu
 * whenever an item is selected/deselected.
 */
FrameListBox::FrameListBox(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size, long style)
  : wxListBox(parent, id, pos, size, 0, nullptr, style)
{
}

// Ensure that the selected item is visible.
void FrameListBox::update() {
  int selection = GetSelection();
  if (selection != wxNOT_FOUND)
    EnsureVisible(selection);
}

// If `event` is a list (de)selection event, broadcast a Frame_MenuUpdate
// event.
bool FrameListBox::TryAfter(wxEvent &event) {
  wxEventType type = event.GetEventType();
  if (type == wxEVT_LISTBOX) {
    wxCommandEvent e(Frame_MenuUpdate, GetId());
    wxPostEvent(GetParent(), e);
  }
  return wxListBox::TryAfter(event);
}
