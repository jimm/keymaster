#include <wx/listctrl.h>
#include "instrument_dialog.h"
#include "instrument_editor.h"
#include "../keymaster.h"

#define CW 48

const char * const COLUMN_HEADERS[] = {
  "Name", "MIDI Port", "Status"
};
const int COLUMN_WIDTHS[] = {
  3*CW, 3*CW, 2*CW
};

wxBEGIN_EVENT_TABLE(InstrumentDialog, wxDialog)
  EVT_LIST_ITEM_ACTIVATED(ID_IL_InputInstruments, InstrumentDialog::edit_input_instrument)
  EVT_LIST_ITEM_ACTIVATED(ID_IL_OutputInstruments, InstrumentDialog::edit_output_instrument)
wxEND_EVENT_TABLE()

int wxCALLBACK inst_list_sort(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData) {
  return ((Instrument *)item1)->name().compare(((Instrument *)item2)->name());
}

InstrumentDialog::InstrumentDialog(wxWindow *parent, KeyMaster *keymaster)
  : wxDialog(parent, wxID_ANY, "Instruments"), km(keymaster)
{     
  inputs = new wxListCtrl(
    this, ID_IL_InputInstruments, wxDefaultPosition, wxSize(600, 150), wxLC_REPORT | wxLC_SINGLE_SEL
  );
  outputs = new wxListCtrl(
    this, ID_IL_OutputInstruments, wxDefaultPosition, wxSize(600, 150), wxLC_REPORT | wxLC_SINGLE_SEL
  );

  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    inputs->InsertColumn(i, COLUMN_HEADERS[i]);
    inputs->SetColumnWidth(i, COLUMN_WIDTHS[i]);
    outputs->InsertColumn(i, COLUMN_HEADERS[i]);
    outputs->SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }

  int i = 0;
  for (auto* inst : km->inputs())
    add_instrument(inputs, inst, i++);
  inputs->SortItems(inst_list_sort, 0);
  i = 0;
  for (auto* inst : km->outputs())
    add_instrument(outputs, inst, i++);
  outputs->SortItems(inst_list_sort, 0);

  wxSizer *buttons = CreateButtonSizer(wxOK);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags label_flags = wxSizerFlags().Align(wxALIGN_LEFT).Expand().Border();
  wxSizerFlags list_flags = wxSizerFlags().Expand().Border();

  sizer->Add(new wxStaticText(this, wxID_ANY, "Inputs"), label_flags);
  sizer->Add(inputs, list_flags);
  sizer->Add(new wxStaticText(this, wxID_ANY, "Outputs"), label_flags);
  sizer->Add(outputs, list_flags);
  sizer->Add(buttons, wxSizerFlags().Expand().Border());

  SetSizerAndFit(sizer);
}

void InstrumentDialog::add_instrument(wxListCtrl *list_box, Instrument *inst, int i) {
  list_box->InsertItem(i, inst->name().c_str());
  list_box->SetItem(i, 1, inst->device_name.c_str());
  list_box->SetItem(i, 2, inst->enabled ? "enabled" : "<disabled>");
  list_box->SetItemData(i, (long)inst);
}

void InstrumentDialog::edit_input_instrument(wxListEvent& event) {
  edit_instrument(inputs, sorted_inputs);
}

void InstrumentDialog::edit_output_instrument(wxListEvent& event) {
  edit_instrument(outputs, sorted_outputs);
}

void InstrumentDialog::edit_instrument(wxListCtrl *list, vector<Instrument *> &instruments) {
  int instrument_num = list->GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (instrument_num == UNDEFINED)
    return;

  Instrument *instrument = (Instrument *)list->GetItemData(instrument_num);
  if (instrument == nullptr)
    return;

  if (InstrumentEditor(this, instrument).ShowModal() == wxID_OK)
    list->SetItem(instrument_num, 0, instrument->name().c_str());
}
