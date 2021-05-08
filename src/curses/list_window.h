#ifndef LIST_WINDOW_H
#define LIST_WINDOW_H

#include <vector>
#include "window.h"

using namespace std;

template <class T>
class ListWindow : public Window {
public:
  vector<T *> *list;
  typename vector<T *>::iterator offset;
  T *curr_item;

  ListWindow(struct rect, const char *);
  ~ListWindow();

  void set_contents(const char *title, vector<T *> *, T *curr_item);
  void draw();

#ifdef DEBUG
  void debug();
#endif
};

#endif /* LIST_WINDOW_H */
