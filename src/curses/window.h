#ifndef SM_WINDOW_H
#define SM_WINDOW_H

#include <string>
#include <ncurses.h>
#include "geometry.h"

using namespace std;

class Window {
public:
  WINDOW *win;
  string title_prefix;
  string title;
  rect rect;

  Window(struct rect r, const char *title_prefix);
  virtual ~Window();

  void move_and_resize(struct rect);
  void draw();
  int visible_height();
  int visible_width();
  void make_fit(string &, int);
  void make_fit(const char *, int, char *);

#ifdef DEBUG
  void debug();
#endif
};

#endif /* SM_WINDOW_H */
