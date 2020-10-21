#include "message_list.h"
#include "../keymaster.h"

MessageList::MessageList(wxWindow *parent, wxWindowID id, wxSize size)
  : FrameListBox(parent, id, wxDefaultPosition, size, wxLB_SINGLE),
    message(nullptr)
{
}

void MessageList::update() {
  KeyMaster *km = KeyMaster_instance();

  Clear();
  wxArrayString names;
  for (auto& message : km->messages())
    names.Add(message->name().c_str());
  if (!names.empty())
    InsertItems(names, 0);
}
