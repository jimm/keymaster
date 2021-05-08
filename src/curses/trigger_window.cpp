#include <stdlib.h>
#include "trigger_window.h"

TriggerWindow::TriggerWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix)
{
  title = "Triggers";
  trigger = 0;
}

TriggerWindow::~TriggerWindow() {
}

void TriggerWindow::set_contents(const char *win_title, Trigger *t) {
  title = win_title;
  trigger = t;
}

void TriggerWindow::draw() {
  Window::draw();
  if (trigger == nullptr)
    return;

  // TODO
}
