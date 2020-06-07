#include <wx/listctrl.h>
#include "instrument_dialog.h"
#include "../keymaster.h"

#define CW 48

const char * const COLUMN_HEADERS[] = {
  "Name", "MIDI Port", "Status"
};
const int COLUMN_WIDTHS[] = {
  3*CW, 3*CW, 2*CW
};

InstrumentDialog::InstrumentDialog(wxWindow *parent, KeyMaster *keymaster)
  : wxDialog(parent, wxID_ANY, "Instruments"), km(keymaster)
{     
  wxListCtrl *inputs = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 150), wxLC_REPORT);
  wxListCtrl *outputs = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 150), wxLC_REPORT);

  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    inputs->InsertColumn(i, COLUMN_HEADERS[i]);
    inputs->SetColumnWidth(i, COLUMN_WIDTHS[i]);
    outputs->InsertColumn(i, COLUMN_HEADERS[i]);
    outputs->SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }

  int i = 0;
  for (auto* inst : km->inputs)
    add_instrument(inputs, inst, i++);
  i = 0;
  for (auto* inst : km->outputs)
    add_instrument(outputs, inst, i++);

  wxSizer *buttons = CreateButtonSizer(wxOK);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags label_flags = wxSizerFlags().Align(wxALIGN_LEFT).Border();
  wxSizerFlags list_flags = wxSizerFlags().Expand().Border();

  sizer->Add(new wxStaticText(this, wxID_ANY, "Inputs"), label_flags);
  sizer->Add(inputs, list_flags);
  sizer->Add(new wxStaticText(this, wxID_ANY, "Outputs"), label_flags);
  sizer->Add(outputs, list_flags);
  sizer->Add(buttons, wxSizerFlags().Expand().Border());

  SetSizerAndFit(sizer);
}

void InstrumentDialog::run() {
  ShowModal();
  Destroy();
}

void InstrumentDialog::add_instrument(wxListCtrl *list_box, Instrument *inst, int i) {
  list_box->InsertItem(i, inst->name.c_str());
  list_box->SetItem(i, 1, inst->port_name.c_str());
  list_box->SetItem(i, 2, inst->enabled ? "enabled" : "<disabled>");
}
