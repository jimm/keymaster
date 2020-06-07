#ifndef TRIGGER_EDITOR_H
#define TRIGGER_EDITOR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "events.h"

enum {
  ID_TE_InputDropdown = 3000,
  ID_TE_MessageText,
  ID_TE_ActionDropdown
};

class KeyMaster;
class Trigger;
class Input;
class wxListCtrl;

class TriggerEditor : public wxDialog {
public:
  TriggerEditor(wxWindow *parent, Trigger *trigger);

private:
  KeyMaster *km;
  Trigger *trigger;
  wxComboBox *lc_key;
  wxComboBox *lc_input;
  wxTextCtrl *tc_trigger_message;
  wxComboBox *lc_action;

  wxWindow *make_key_dropdown(wxWindow *parent);
  wxWindow *make_input_dropdown(wxWindow *parent);
  wxWindow *make_action_dropdown(wxWindow *parent);

  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* TRIGGER_EDITOR_H */
