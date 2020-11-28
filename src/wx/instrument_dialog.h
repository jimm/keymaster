#ifndef INSTRUMENT_DIALOG_H
#define INSTRUMENT_DIALOG_H

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif

using namespace std;

enum {
  ID_IL_InputInstruments = 9000,
  ID_IL_OutputInstruments
};

class KeyMaster;
class Instrument;

class InstrumentDialog : public wxDialog {
public:
  InstrumentDialog(wxWindow *parent, KeyMaster *km);
  void run();

private:
  KeyMaster *km;
  wxListCtrl *inputs;
  wxListCtrl *outputs;
  vector<Instrument *>sorted_inputs;
  vector<Instrument *>sorted_outputs;

  void add_instrument(wxListCtrl *list_box, Instrument *inst, int i);

  void edit_input_instrument(wxListEvent& event);
  void edit_output_instrument(wxListEvent& event);
  void edit_instrument(wxListCtrl *list, vector<Instrument *> &instruments);

  wxDECLARE_EVENT_TABLE();
};

#endif /* INSTRUMENT_DIALOG_H */