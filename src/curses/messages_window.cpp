#include <stdlib.h>
#include "messages_window.h"

MessagesWindow::MessagesWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix)
{
  title = "Messages";
}

MessagesWindow::~MessagesWindow() {
}

void MessagesWindow::draw() {
  Window::draw();

  // TODO get from km instance
}
