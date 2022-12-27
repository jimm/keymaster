#ifndef SET_LIST_LIST_BOX_H
#define SET_LIST_LIST_BOX_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "frame_list_box.h"

class SetListListBox : public FrameListBox {
public:
  SetListListBox(wxWindow *parent, wxWindowID id);

  void update();
  void jump();
};

#endif /* SET_LIST_LIST_BOX_H */
