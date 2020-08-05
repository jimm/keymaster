#include "song_editor.h"
#include "macros.h"
#include "../formatter.h"
#include "../keymaster.h"
#include "../song.h"

#define BPM_WIDTH 52
#define BPM_HEIGHT TEXT_EDIT_HEIGHT

wxBEGIN_EVENT_TABLE(SongEditor, wxDialog)
  EVT_BUTTON(wxID_OK, SongEditor::save)
wxEND_EVENT_TABLE()

SongEditor::SongEditor(wxWindow *parent, Song *song_ptr)
: wxDialog(parent, wxID_ANY, "Song Editor", wxDefaultPosition),
    song(song_ptr)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);
  sizer->Add(make_name_panel(this), panel_flags);
  sizer->Add(make_clock_panel(this), panel_flags);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_flags);
  SetSizerAndFit(sizer);
}

wxWindow *SongEditor::make_name_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags panel_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Expand().Border(wxTOP|wxLEFT|wxRIGHT);

  sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Name")), panel_flags);
  name_text = new wxTextCtrl(p, ID_SE_Name, song->name, wxDefaultPosition, NAME_CTRL_SIZE);
  sizer->Add(name_text, panel_flags);

  p->SetSizerAndFit(sizer);
  return p;
}

wxWindow *SongEditor::make_clock_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags panel_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Expand().Border(wxTOP|wxLEFT|wxRIGHT);

  char buf[16];
  format_float(song->bpm, buf);
  text_bpm = new wxTextCtrl(p, ID_SE_BPM, buf, wxDefaultPosition, wxSize(BPM_WIDTH, BPM_HEIGHT));
  field_sizer->Add(text_bpm, panel_flags);
  field_sizer->Add(new wxStaticText(p, wxID_ANY, "BPM"), panel_flags);

  cb_clock_start = new wxCheckBox(p, ID_SE_StartClock, "Start Clock");
  cb_clock_start->SetValue(song->clock_on_at_start);
  field_sizer->Add(cb_clock_start, panel_flags);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Clock")));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

void SongEditor::save(wxCommandEvent& _) {
  song->name = name_text->GetLineText(0);
  float bpm = atof(text_bpm->GetValue());
  if (bpm != 0)
    song->bpm = bpm;
  song->clock_on_at_start = cb_clock_start->IsChecked();

  EndModal(wxID_OK);
}
