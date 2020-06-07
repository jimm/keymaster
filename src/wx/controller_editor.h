#ifndef CONTROLLER_EDITOR_H
#define CONTROLLER_EDITOR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "events.h"

using namespace std;

enum {
  ID_CMAP_CCNumber = 5000,
  ID_CMAP_TranslatedNumber,
  ID_CMAP_MinIn,
  ID_CMAP_MaxIn,
  ID_CMAP_MinOut,
  ID_CMAP_MaxOut,
  ID_CMAP_PassThrough0,
  ID_CMAP_PassThrough127,
  ID_CMAP_Filtered
};

class KeyMaster;
class Instrument;
class Connection;
class Controller;
class wxListCtrl;
class wxListEvent;

class ControllerEditor : public wxDialog {
public:
  ControllerEditor(wxWindow *parent, Connection *connection, Controller *controller);

private:
  Connection *connection;
  Controller *controller;
  int orig_cc_num;
  wxComboBox *cb_cc_number;
  wxComboBox *cb_xlated_number;
  wxTextCtrl *tc_min_in;
  wxTextCtrl *tc_max_in;
  wxTextCtrl *tc_min_out;
  wxTextCtrl *tc_max_out;
  wxCheckBox *cb_pass_through_0;
  wxCheckBox *cb_pass_through_127;
  wxCheckBox *cb_filtered;

  wxWindow *make_numbers_panel(wxWindow *parent);
  wxWindow *make_val_mapping_panel(wxWindow *parent);
  wxWindow *make_filtered_panel(wxWindow *parent);
  wxComboBox *make_cc_number_dropdown(wxWindow *parent, wxWindowID id,
                                      int curr_val, bool filter_out_existing);

  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* CONTROLLER_EDITOR_H */
