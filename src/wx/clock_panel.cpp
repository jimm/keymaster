#include "clock_panel.h"
#include "../keymaster.h"
#include "../clock.h"
#include "clock_on.xpm"
#include "clock_off.xpm"

#define BPM_WIDTH 48
#define BPM_HEIGHT 24
#define TIMER_MILLISECS 10
#define TIMER_ID 20000

enum {
  ID_ClockBPM = 200,
  ID_ClockToggle
};

wxBEGIN_EVENT_TABLE(ClockPanel, wxPanel)
  EVT_TEXT(ID_ClockBPM, ClockPanel::set_clock_bpm)
  EVT_BUTTON(ID_ClockToggle, ClockPanel::toggle_clock)
  EVT_TIMER(TIMER_ID, ClockPanel::on_timer)
wxEND_EVENT_TABLE()


ClockPanel::ClockPanel(wxWindow *parent)
: wxPanel(parent, wxID_ANY), timer(this, TIMER_ID),
  clock_on_bitmap(clock_on_xpm), clock_off_bitmap(clock_off_xpm)
{     
  lc_clock_bpm = new wxTextCtrl(this, ID_ClockBPM, "120", wxDefaultPosition,
                                wxSize(BPM_WIDTH, BPM_HEIGHT));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, "Clock"), wxSizerFlags().Align(wxALIGN_LEFT));

  wxBoxSizer *control_sizer = new wxBoxSizer(wxHORIZONTAL);
  control_sizer->Add(lc_clock_bpm, wxSizerFlags(1).Expand().Border());
  control_sizer->Add(new wxStaticText(this, wxID_ANY, "BPM"), wxSizerFlags().Align(wxALIGN_LEFT));

  onoff_button = new wxBitmapButton(this, ID_ClockToggle, clock_off_bitmap);
  control_sizer->Add(onoff_button, wxSizerFlags().Align(wxALIGN_RIGHT));

  sizer->Add(control_sizer);

  SetSizerAndFit(sizer);

  KeyMaster *km = KeyMaster_instance();
  if (km != nullptr)
    km->clock.set_monitor(this);

  update();
  timer.Start(TIMER_MILLISECS);
}

ClockPanel::~ClockPanel() {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr)
    return;
  km->clock.set_monitor(nullptr);
}

void ClockPanel::monitor_bpm(int bpm) {
  lc_clock_bpm->SetValue(wxString::Format("%f", bpm));
}

void ClockPanel::monitor_start() {
  onoff_button->SetBitmap(clock_on_bitmap);
}

void ClockPanel::monitor_stop() {
  onoff_button->SetBitmap(clock_off_bitmap);
}

void ClockPanel::monitor_beat() {
  // TODO turn on flashing light for a short while
}

void ClockPanel::set_clock_bpm(wxCommandEvent& event) {
  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr)
    return;
  int bpm = atof(lc_clock_bpm->GetValue());
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
  Clock &clock = km->clock;

  lc_clock_bpm->SetValue(wxString::Format("%f", clock.bpm()));
  onoff_button->SetBitmap(clock.is_running() ? clock_on_bitmap : clock_off_bitmap);
}
