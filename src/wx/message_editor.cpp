#include <wx/gbsizer.h>
#include "message_editor.h"
#include "macros.h"
#include "../keymaster.h"
#include "../message.h"
#include "../formatter.h"

#define WIDTH 200
#define HEIGHT 300

wxBEGIN_EVENT_TABLE(MessageEditor, wxDialog)
  EVT_BUTTON(wxID_OK, MessageEditor::save)
wxEND_EVENT_TABLE()

MessageEditor::MessageEditor(wxWindow *parent, Message *m)
  : wxDialog(parent, wxID_ANY, "Message Editor", wxDefaultPosition),
    km(KeyMaster_instance()), message(m)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);

  sizer->Add(make_name_panel(this), panel_flags);
  message_text = new wxTextCtrl(this, ID_ME_MessageText, message_to_text(),
                                wxDefaultPosition, wxSize(WIDTH, HEIGHT),
                                wxTE_MULTILINE);
  sizer->Add(message_text,
             wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT));

  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_flags);

  SetSizerAndFit(sizer);
}

wxWindow *MessageEditor::make_name_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags center_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

  sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Name")), center_flags);
  name_text = new wxTextCtrl(p, ID_ME_Name, message->name, wxDefaultPosition, NAME_CTRL_SIZE);
  sizer->Add(name_text, center_flags);

  p->SetSizerAndFit(sizer);
  return p;
}

wxString MessageEditor::message_to_text() {
  return wxString(message->to_editable_string());
}

void MessageEditor::save(wxCommandEvent& _) {
  message->name = name_text->GetLineText(0);
  wxString text = message_text->GetValue();
  message->from_editable_string(text.ToStdString());
  EndModal(wxID_OK);
}
