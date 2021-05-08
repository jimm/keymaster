#include <sstream>
#include "midi_monitor_window.h"
#include "../formatter.h"

MIDIMonitorWindow::MIDIMonitorWindow(struct rect r, KeyMaster *patchmaster)
  : Window(r, nullptr), km(patchmaster)
{
  title = "MIDI Monitor (press 'm' to close)";
  for (auto *input : km->inputs())
    input->add_observer(this);
  for (auto *output : km->outputs())
    output->add_observer(this);
}

MIDIMonitorWindow::~MIDIMonitorWindow() {
  for (auto *input : km->inputs())
    input->remove_observer(this);
  for (auto *output : km->outputs())
    output->remove_observer(this);
}

void MIDIMonitorWindow::update(Observable *o, void *arg) {
  PmMessage msg = (PmMessage)(long)arg;
  if (static_cast<Instrument *>(o)->is_input())
    monitor_input(static_cast<Input *>(o), msg);
  else
    monitor_output(static_cast<Output *>(o), msg);
}

void MIDIMonitorWindow::monitor_input(Input *input, PmMessage msg) {
  fprintf(stderr, "monitor_input %s, msg %08x\n", input->name().c_str(), msg);
  add_message(input_lines, input->name(), msg);
  draw();
  wnoutrefresh(win);
  doupdate();
}

void MIDIMonitorWindow::monitor_output(Output *output, PmMessage msg) {
  fprintf(stderr, "monitor_output %s, msg %08x\n", output->name().c_str(), msg);
  add_message(output_lines, output->name(), msg);
  draw();
  wnoutrefresh(win);
  doupdate();
}

void MIDIMonitorWindow::add_message(deque<string> &lines, string sym, PmMessage msg) {
  if (lines.size() > getmaxx(stdscr) - 2)
    lines.pop_front();

  unsigned char bytes[3] = {
    (unsigned char)Pm_MessageStatus(msg),
    (unsigned char)Pm_MessageData1(msg),
    (unsigned char)Pm_MessageData2(msg)
  };
  ostringstream ostr;
  ostr << sym
       << '\t' << bytes_to_hex(bytes + 0, 1)
       << ' ' << bytes_to_hex(bytes + 1, 1)
       << ' ' << bytes_to_hex(bytes + 2, 1);
  lines.push_back(ostr.str());
}

void MIDIMonitorWindow::draw() {
  Window::draw();
  int row = 1;
  int col = visible_width() / 2;
  while (row <= visible_height()) {
    wmove(win, row++, col);
    waddch(win, ACS_VLINE);
  }
  wmove(win, row, col);
  waddch(win, ACS_BTEE);

  draw_lines(input_lines, 1);
  draw_lines(output_lines, visible_width() / 2 + 1);
  refresh();
}

void MIDIMonitorWindow::draw_lines(deque<string> &lines, int col) {
  int start_line_num = 0;
  if (lines.size() > visible_height())
    start_line_num = lines.size() - visible_height();
  int row = 1;
  int max_width = visible_width() / 2 - 4;
  for (int i = start_line_num; i < lines.size(); ++i) {
    string line = lines.at(i);
    make_fit(line, max_width);
    wmove(win, row++, col);
    waddstr(win, line.c_str());
  }
}
