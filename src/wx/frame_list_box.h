#ifndef FRAME_LIST_BOX_H
#define FRAME_LIST_BOX_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif

class FrameListBox : public wxListBox {
public:
  FrameListBox(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size, long style);

  virtual void update();

protected:
  virtual bool TryAfter(wxEvent &event);
};

#endif /* FRAME_LIST_BOX_H */
