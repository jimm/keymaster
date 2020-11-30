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
  wxButton *start_button;
  wxButton *continue_button;
  wxButton *stop_button;
  float display_bpm;

  void set_clock_bpm(wxCommandEvent& event);
  void start_clock(wxCommandEvent& event);
  void continue_clock(wxCommandEvent& event);
  void stop_clock(wxCommandEvent& event);

  void update_clock_buttons(bool start, bool cont, bool stop);

  wxDECLARE_EVENT_TABLE();
};

#endif /* MONITOR_H */
