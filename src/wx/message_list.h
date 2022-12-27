#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "frame_list_box.h"

class Message;

class MessageList : public FrameListBox {
public:
  MessageList(wxWindow *parent, wxWindowID id);

  void update();

private:
  Message *message;
};

#endif /* MESSAGE_BOX_H */
