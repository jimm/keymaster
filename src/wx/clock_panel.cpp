#include "clock_panel.h"
#include "macros.h"
#include "../keymaster.h"
#include "../formatter.h"
#include "../clock.h"

#define BPM_WIDTH 52
#define BPM_HEIGHT 20
#define TIMER_MILLISECS 10
#define TIMER_ID 20000

enum {
  ID_ClockBPM = 200,
  ID_ClockToggle
};

wxBEGIN_EVENT_TABLE(ClockPanel, wxPanel)
  EVT_TEXT_ENTER(ID_ClockBPM, ClockPanel::set_clock_bpm)
  EVT_TOGGLEBUTTON(ID_ClockToggle, ClockPanel::toggle_clock)
  EVT_TIMER(TIMER_ID, ClockPanel::on_timer)
wxEND_EVENT_TABLE()


ClockPanel::ClockPanel(wxWindow *parent)
: wxPanel(parent, wxID_ANY), timer(this, TIMER_ID), display_bpm(0)
{     
  lc_clock_bpm = new wxTextCtrl(this, ID_ClockBPM, "", wxDefaultPosition,
                                wxSize(BPM_WIDTH, BPM_HEIGHT), wxTE_PROCESS_ENTER);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, TITLE_STR("Clock")), wxSizerFlags().Align(wxALIGN_LEFT));

  wxBoxSizer *control_sizer = new wxBoxSizer(wxHORIZONTAL);
  control_sizer->Add(lc_clock_bpm, wxSizerFlags(1).Expand().Border().Align(wxALIGN_CENTER_VERTICAL));
  // Spaces after "BPM" are for visual spacing
  control_sizer->Add(new wxStaticText(this, wxID_ANY, TITLE_STR("BPM")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

  onoff_button = new wxToggleButton(this, ID_ClockToggle, "off");
  control_sizer->Add(onoff_button, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

  sizer->Add(control_sizer);

  SetSizerAndFit(sizer);

  KeyMaster *km = KeyMaster_instance();
  if (km != nullptr)
    km->clock().add_observer(this);

  update();
  timer.Start(TIMER_MILLISECS);
}

void ClockPanel::update(Observable *o, void *arg) {
  ClockChange clock_update = (ClockChange)(long)arg;
  switch (clock_update) {
  case ClockChangeBpm:
    lc_clock_bpm->SetValue(wxString::Format("%f", KeyMaster_instance()->clock().bpm()));
    break;
  case ClockChangeStart:
    onoff_button->SetLabelText("on");
    onoff_button->SetValue(true);
    break;
  case ClockChangeStop:
    onoff_button->SetLabelText("off");
    onoff_button->SetValue(false);
    break;
  case ClockChangeBeat:
    // TODO turn on flashing light for a short while
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

void ClockPanel::toggle_clock(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr)
    return;
  km->toggle_clock();
  update();
}

void ClockPanel::on_timer(wxTimerEvent &event) {
  Refresh();
  Update();
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

  onoff_button->SetLabelText(clock.is_running() ? "on" : "off");
  onoff_button->SetValue(clock.is_running());
}
