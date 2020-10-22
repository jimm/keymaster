#ifndef CLOCK_PANEL_H
#define CLOCK_PANEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include <wx/tglbtn.h>
#include "../observer.h"

class ClockPanel : public wxPanel, public Observer {
public:
  ClockPanel(wxWindow *parent);

  virtual void update(Observable *o, void *arg);
  void update();

private:
  wxTextCtrl *lc_clock_bpm;
  wxToggleButton *onoff_button;
  wxTimer timer;
  float display_bpm;

  void set_clock_bpm(wxCommandEvent& event);
  void toggle_clock(wxCommandEvent& event);

  void on_timer(wxTimerEvent &event);

  wxDECLARE_EVENT_TABLE();
};

#endif /* MONITOR_H */
