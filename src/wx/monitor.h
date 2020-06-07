#ifndef MONITOR_H
#define MONITOR_H

#include <deque>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "../midi_monitor.h"

using namespace std;

class KeyMaster;
class Instrument;

class MonitorMessage {
public:
  Instrument *instrument;
  PmMessage message;

  MonitorMessage(Instrument *inst, PmMessage msg) : instrument(inst), message(msg) {}
};

class Monitor : public wxFrame, public MIDIMonitor {
public:
  Monitor();
  virtual ~Monitor();

  virtual void monitor_input(Input *input, PmMessage msg);
  virtual void monitor_output(Output *output, PmMessage msg);

private:
  wxListCtrl *input_list;
  wxListCtrl *output_list;
  deque<MonitorMessage> input_messages;
  deque<MonitorMessage> output_messages;
  wxTimer timer;
  bool modified;

  wxWindow *make_input_panel(wxWindow *parent);
  wxWindow *make_output_panel(wxWindow *parent);
  wxWindow *make_panel(wxWindow *parent, const char * const title, wxListCtrl **list_ptr);
  void add_message(Instrument *inst, wxListCtrl *list, PmMessage msg, deque<MonitorMessage> &message_list);

  void on_timer(wxTimerEvent &event);
  void update_list(wxListCtrl *list, deque<MonitorMessage> &message_list);

  wxDECLARE_EVENT_TABLE();
};

#endif /* MONITOR_H */
