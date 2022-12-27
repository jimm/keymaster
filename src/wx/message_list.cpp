#include "message_list.h"
#include "../keymaster.h"

MessageList::MessageList(wxWindow *parent, wxWindowID id)
  : FrameListBox(parent, id),
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
