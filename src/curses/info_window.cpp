#include <stdlib.h>
#include <string.h>
#include "info_window.h"
#include "help_window.h"

InfoWindow::InfoWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix), text_lines(nullptr), contents_lines(nullptr)
{
  help_lines = info_window_text_to_lines(help_window_read_help());
  display_list = help_lines;    /* do not delete this */
}

InfoWindow::~InfoWindow() {
  info_window_free_lines(help_lines);
  info_window_free_lines(contents_lines);
}

void InfoWindow::set_contents(vector<char *> *text_lines_v) {
  if (text_lines_v == nullptr || text_lines_v->empty()) {
    title = "KeyMaster Help";
    display_list = help_lines;
    return;
  }

  title = "Song Notes";
  text_lines = text_lines_v;
  display_list = text_lines_v;
}

void InfoWindow::set_contents(string *text_string) {
  if (text_string == nullptr || text_string->empty()) {
    title = "KeyMaster Help";
    display_list = help_lines;
    return;
  }

  char *buf = (char *)malloc(text_string->size() + 1);
  strncpy(buf, text_string->c_str(), text_string->size());
  buf[text_string->size()] = 0;

  info_window_free_lines(contents_lines);

  title = "Song Notes";
  contents_lines = info_window_text_to_lines(buf);
  display_list = contents_lines;
}

void InfoWindow::draw() {
  char fitted[BUFSIZ];
  int vis_height = visible_height();
  bool must_truncate = display_list->size() >= vis_height;

  Window::draw();
  int row = 1;
  for (auto& line : *display_list) {
    wmove(win, row++, 1);
    if (must_truncate && row > vis_height) {
      waddstr(win, "...");
      break;
    }
    make_fit(line, 1, fitted);
    waddstr(win, fitted);
  }
}

/*
 * Splits `text` into lines and returns a list containing the lines. When
 * you are done with the list, only the first entry should be freed.
 */
vector<char *> *info_window_text_to_lines(const char *text) {
  vector<char *> *l = new vector<char *>();

  char *line;
  char *s = strdup(text);
  while ((line = strsep(&s, "\n")) != NULL)
    l->push_back(line);

  return l;
}

void info_window_free_lines(vector<char *> *lines) {
  if (lines == nullptr)
    return;

  if (!lines->empty())
    free((*lines)[0]);
  delete lines;
}
