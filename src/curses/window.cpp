#include <stdlib.h>
#include <string.h>
#include "window.h"

Window::Window(struct rect r, const char *title_prefix_str)
{
  if (title_prefix_str != nullptr)
    title_prefix = title_prefix_str;
  title = "";
  win = newwin(r.height, r.width, r.row, r.col);
  rect = r;
}

Window::~Window() {
  delwin(win);
}

void Window::move_and_resize(struct rect r) {
  rect = r;
  mvwin(win, r.row, r.col);
  wresize(win, r.height, r.width);
}

void Window::draw() {
  wclear(win);
  box(win, ACS_VLINE, ACS_HLINE);
  if (title_prefix.length() == 0 && title.length() == 0)
    return;

  string win_title = " ";
  if (title_prefix.length() > 0) {
    win_title += title_prefix;
    win_title += ": ";
  }
  if (title.length() > 0)
    win_title += title;
  win_title += " ";
  make_fit(win_title, 0);

  wmove(win, 0, 1);
  wattron(win, A_REVERSE);
  waddstr(win, win_title.c_str());
  wattroff(win, A_REVERSE);
}

int Window::visible_height() {
  return getmaxy(win) - 2;
}

int Window::visible_width() {
  return getmaxx(win) - 2;
}

/*
 * MODIFIES str by fitting it to the maximum window display width (which is
 * the window width minus two for the borders) minus `reduce_max_len_by`.
 */
void Window::make_fit(string &str, int reduce_max_len_by) {
  int w_maxlen = getmaxx(win) - (2 + reduce_max_len_by);
  if (str.length() > w_maxlen)
    str.resize(w_maxlen, ' ');
}

/*
 * Copies str into outbuf to the maximum window display width (which is the
 * window width minus two for the borders) minus `reduce_max_len_by`.
 */
void Window::make_fit(const char *str, int reduce_max_len_by, char *outbuf) {
  int len = strlen(str);
  int w_maxlen = getmaxx(win) - (2 + reduce_max_len_by);
  int newlen = len < w_maxlen ? len : w_maxlen;
  strncpy(outbuf, str, newlen);
  outbuf[newlen] = 0;
}

#ifdef DEBUG

void Window::debug() {
  fprintf(stderr, "window %p, win %p, title_prefix %s, title %s\n",
          this, win, title_prefix.c_str(), title.c_str());
}

#endif
