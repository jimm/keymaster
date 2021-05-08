#include "error.h"
#include "curses/gui.h"

void error_message(const char * const msg) {
  fprintf(stderr, "%s\n", msg);
  GUI *gui = gui_instance();
  if (gui != nullptr) {
    gui->show_message(msg);
    gui->clear_message_after(30);
  }
}
