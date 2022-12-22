#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <vector>
#include "window.h"

class HelpWindow : public Window {
public:
  vector<char *> *lines;

  HelpWindow(struct rect, const char *);
  ~HelpWindow();

  void draw();
};

const char *help_window_read_help(); // used by info_window

#endif /* HELP_WINDOW_H */
