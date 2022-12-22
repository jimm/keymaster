#ifndef MIDI_MONITOR_WINDOW_H
#define MIDI_MONITOR_WINDOW_H

#include <deque>
#include "window.h"
#include "../keymaster.h"
#include "../input.h"
#include "../output.h"

class MIDIMonitorWindow : public Window, public Observer {
public:
  MIDIMonitorWindow(struct rect, KeyMaster *);
  virtual ~MIDIMonitorWindow();

  virtual void update(Observable *o, void *arg);

  void draw();

private:
  KeyMaster *km;
  deque<string> input_lines;
  deque<string> output_lines;

  void monitor_input(Input *input, PmMessage msg);
  void monitor_output(Output *output, PmMessage msg);
  void add_message(deque<string> &lines, string sym, PmMessage msg);

  void draw_lines(deque<string> &lines, int col);
};

#endif /* MIDI_MONITOR_WINDOW_H */
