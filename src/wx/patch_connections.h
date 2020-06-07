#ifndef PATCH_CONNECTIONS_H
#define PATCH_CONNECTIONS_H

#include "wx/listctrl.h"
#include "frame_list_ctrl.h"
#include "../patch.h"

class PatchConnections : public FrameListCtrl {
public:
  PatchConnections(wxWindow *parent, wxWindowID id);

  Connection *selected();       // may return nullptr
  void update();

private:

  void set_headers();
};

#endif /* PATCH_CONNECTIONS_H */
