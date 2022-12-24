#include <stdlib.h>
#include <ncurses.h>
#include "prompt_window.h"

PromptWindow::PromptWindow(const char *title_str)
  : Window(geom_prompt_rect(), nullptr)
{
  title = title_str;
}

PromptWindow::~PromptWindow() {
}

string PromptWindow::gets() {
  draw();
  return read_string();
}

void PromptWindow::draw() {
  Window::draw();

  wmove(win, 0, 1);
  wattron(win, A_REVERSE);
  waddch(win, ' ');
  waddstr(win, title.c_str());
  waddch(win, ' ');
  wattroff(win, A_REVERSE);

  wmove(win, 1, 1);
  refresh();
}

string PromptWindow::read_string() {
  char *str = (char *)malloc(BUFSIZ);

  nocbreak();
  echo();
  curs_set(1);
  wgetnstr(win, str, BUFSIZ);
  curs_set(0);
  noecho();
  cbreak();

  return string(str);
}
