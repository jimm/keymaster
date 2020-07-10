#ifndef CLOCK_PANEL_H
#define CLOCK_PANEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "../clock_monitor.h"

class ClockPanel : public wxPanel, public ClockMonitor {
public:
  ClockPanel(wxWindow *parent);
  virtual ~ClockPanel();

  void monitor_bpm(int bpm);
  void monitor_start();
  void monitor_stop();
  void monitor_beat();

  void update();

private:
  wxTextCtrl *lc_clock_bpm;
  wxBitmapButton *onoff_button;
  wxTimer timer;
  wxBitmap clock_on_bitmap;
  wxBitmap clock_off_bitmap;

  void set_clock_bpm(wxCommandEvent& event);
  void toggle_clock(wxCommandEvent& event);

  void on_timer(wxTimerEvent &event);

  wxDECLARE_EVENT_TABLE();
};

#endif /* MONITOR_H */
