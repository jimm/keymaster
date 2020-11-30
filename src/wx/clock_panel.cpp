#include "clock_panel.h"
#include "macros.h"
#include "../keymaster.h"
#include "../formatter.h"
#include "../clock.h"

#define BPM_WIDTH 52
#define BPM_HEIGHT 20

enum {
  ID_ClockBPM = 200,
  ID_ClockStart,
  ID_ClockContinue,
  ID_ClockStop
};

wxBEGIN_EVENT_TABLE(ClockPanel, wxPanel)
  EVT_TEXT_ENTER(ID_ClockBPM, ClockPanel::set_clock_bpm)
  EVT_BUTTON(ID_ClockStart, ClockPanel::start_clock)
  EVT_BUTTON(ID_ClockContinue, ClockPanel::continue_clock)
  EVT_BUTTON(ID_ClockStop, ClockPanel::stop_clock)
wxEND_EVENT_TABLE()


ClockPanel::ClockPanel(wxWindow *parent)
  : wxPanel(parent, wxID_ANY), display_bpm(0)
{     
  lc_clock_bpm = new wxTextCtrl(this, ID_ClockBPM, "", wxDefaultPosition,
                                wxSize(BPM_WIDTH, BPM_HEIGHT), wxTE_PROCESS_ENTER);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, TITLE_STR("Clock")), wxSizerFlags().Align(wxALIGN_LEFT));

  wxBoxSizer *control_sizer = new wxBoxSizer(wxHORIZONTAL);
  control_sizer->Add(lc_clock_bpm, wxSizerFlags(1).Expand().Border().Align(wxALIGN_CENTER_VERTICAL));
  // Spaces after "BPM" are for visual spacing
  control_sizer->Add(new wxStaticText(this, wxID_ANY, TITLE_STR("BPM")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

  start_button = new wxButton(this, ID_ClockStart, "Start");
  control_sizer->Add(start_button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
  continue_button = new wxButton(this, ID_ClockContinue, "Cont.");
  control_sizer->Add(continue_button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
  stop_button = new wxButton(this, ID_ClockStop, "Stop");
  control_sizer->Add(stop_button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
  update_clock_buttons(false, false, true);

  sizer->Add(control_sizer);

  SetSizerAndFit(sizer);

  KeyMaster *km = KeyMaster_instance();
  if (km != nullptr)
    km->clock().add_observer(this);

  update();
}

void ClockPanel::update(Observable *o, void *arg) {
  ClockChange clock_update = (ClockChange)(long)arg;
  switch (clock_update) {
  case ClockChangeBpm:
    char buf[16];
    format_float(KeyMaster_instance()->clock().bpm(), buf);
    lc_clock_bpm->SetValue(buf);
    break;
  case ClockChangeStart:
    update_clock_buttons(true, false, false);
    break;
  case ClockChangeContinue:
    update_clock_buttons(false, true, false);
    break;
  case ClockChangeStop:
    update_clock_buttons(false, false, true);
    break;
  }
}

void ClockPanel::set_clock_bpm(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr)
    return;
  float bpm = atof(lc_clock_bpm->GetValue());
  if (bpm != 0)
    km->set_clock_bpm(bpm);
}

void ClockPanel::start_clock(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr || km->clock_is_running())
    return;
  km->start_clock();
  // Clock will fire update we're observing
}

void ClockPanel::continue_clock(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr || km->clock_is_running())
    return;
  km->continue_clock();
  // Clock will fire update we're observing
}

void ClockPanel::stop_clock(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr || !km->clock_is_running())
    return;
  km->stop_clock();
  // Clock will fire update we're observing
}

void ClockPanel::update() {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr)
    return;
  Clock &clock = km->clock();

  float bpm = clock.bpm();
  if (bpm != display_bpm) {
    // Only format the BPM string when the value has changed
    char buf[16];
    format_float(clock.bpm(), buf);
    lc_clock_bpm->SetValue(buf);
    display_bpm = bpm;
  }
}

void ClockPanel::update_clock_buttons(bool start, bool cont, bool stop) {
  start_button->Enable(stop);
  continue_button->Enable(stop);
  stop_button->Enable(!stop);
}
