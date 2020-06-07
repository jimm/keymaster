#ifndef CONTROLLER_MAPPINGS_H
#define CONTROLLER_MAPPINGS_H

#include "wx/listctrl.h"
#include "../controller.h"

class Connection;

class ControllerMappings : public wxListCtrl {
public:
  ControllerMappings(wxWindow *parent, wxWindowID id, Connection *conn);

  void update();

private:
  Connection *connection;

  void set_headers();
};

#endif /* CONTROLLER_MAPPINGS_H */
