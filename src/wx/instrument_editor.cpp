#include "instrument_editor.h"
#include "macros.h"
#include "../formatter.h"
#include "../keymaster.h"
#include "../instrument.h"

wxBEGIN_EVENT_TABLE(InstrumentEditor, wxDialog)
  EVT_BUTTON(wxID_OK, InstrumentEditor::save)
wxEND_EVENT_TABLE()

InstrumentEditor::InstrumentEditor(wxWindow *parent, Instrument *instrument_ptr)
  : wxDialog(parent, wxID_ANY, "Instrument Editor", wxDefaultPosition),
    instrument(instrument_ptr)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border();
  sizer->Add(new wxStaticText(this, wxID_ANY, instrument->device_name), panel_flags);
  sizer->Add(make_name_panel(this), panel_flags);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_flags);
  SetSizerAndFit(sizer);
}

wxWindow *InstrumentEditor::make_name_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border();

  sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Name")), panel_flags);
  name_text = new wxTextCtrl(p, ID_IN_Name, instrument->name(), wxDefaultPosition, NAME_CTRL_SIZE);
  sizer->Add(name_text, panel_flags);

  p->SetSizerAndFit(sizer);
  return p;
}

void InstrumentEditor::save(wxCommandEvent& _) {
  instrument->set_name(name_text->GetLineText(0));

  EndModal(wxID_OK);
}
