#include "frame_list_ctrl.h"
#include "events.h"

/*
 * A FrameListCtrl is a wxListCtrl that tells the frame to update its menu
 * whenever an item is selected/deselected.
 */
FrameListCtrl::FrameListCtrl(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size, long style)
  : wxListCtrl(parent, id, pos, size, style)
{
}

// If `event` is a list (de)selection event, broadcast a Frame_MenuUpdate
// event.
bool FrameListCtrl::TryAfter(wxEvent &event) {
  wxEventType type = event.GetEventType();
  if (type == wxEVT_LIST_ITEM_SELECTED || type == wxEVT_LIST_ITEM_DESELECTED) {
    wxCommandEvent e(Frame_MenuUpdate, GetId());
    wxPostEvent(GetParent(), e);
  }
  return wxListCtrl::TryAfter(event);
}
