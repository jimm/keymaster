#ifndef FRAME_LIST_CTRL_H
#define FRAME_LIST_CTRL_H

#include "wx/listctrl.h"

class FrameListCtrl : public wxListCtrl {
public:
  FrameListCtrl(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size,
                long style);

protected:
  virtual bool TryAfter(wxEvent &event);
};

#endif /* FRAME_LIST_CTRL_H */
