#ifndef INSTRUMENT_EDITOR_H
#define INSTRUMENT_EDITOR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "event.h"

using namespace std;

enum {
  ID_IN_Name = 8000
};

class Instrument;

class InstrumentEditor : public wxDialog {
public:
  InstrumentEditor(wxWindow *parent, Instrument *instrument);

private:
  Instrument *instrument;
  wxTextCtrl *name_text;

  wxWindow *make_name_panel(wxWindow *parent);

  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* INSTRUMENT_EDITOR_H */
