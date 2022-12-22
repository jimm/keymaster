#include <stdlib.h>
#include <string.h>
#include "help_window.h"
#include "info_window.h"

HelpWindow::HelpWindow(struct rect r, const char *win_title)
  : Window(r, nullptr)
{
  title = win_title;
  lines = info_window_text_to_lines(help_window_read_help());
}

HelpWindow::~HelpWindow() {
  info_window_free_lines(lines);
}

void HelpWindow::draw() {
  Window::draw();
  int row = 1;
  for (auto& line : *lines) {
    wmove(win, row++, 1);
    waddstr(win, line);
  }
}

const char *help_window_read_help() {
  return
"j, down, space  - Next patch\n" \
"k, up           - Prev patch\n" \
"n, right        - Next song\n" \
"p, left         - Prev song\n" \
"\n" \
"g    - Goto song\n" \
"t    - Goto song list\n" \
"\n" \
"h, ? - Help\n" \
"ESC  - Panic (ESC ESC sends note-offs)\n" \
"\n" \
"m    - MIDI monitor\n" \
"\n" \
"v    - toggle view\n" \
"\n" \
"l    - load\n" \
"r    - reload\n" \
"s    - save\n" \
"\n" \
"q    - quit";
}
