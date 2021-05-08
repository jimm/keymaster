#include <stdlib.h>
#include "list_window.h"

template <class T>
ListWindow<T>::ListWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix), list(nullptr), curr_item(nullptr)
{
}

template <class T>
ListWindow<T>::~ListWindow() {
}

template <class T>
void ListWindow<T>::set_contents(const char *title_str, vector<T *> *l,
                                 T *curr_item_ptr)
{
  title = title_str ? title_str : "";
  if (list != l) {
    list = l;
    offset = list->begin();
  }
  curr_item = curr_item_ptr;
}

template <class T>
void ListWindow<T>::draw() {
  Window::draw();
  if (list == nullptr || curr_item == nullptr)
    return;

  typename vector<T *>::iterator curr_index = find(list->begin(), list->end(), curr_item);
  int vis_height = visible_height();

  if (curr_index < offset)
    offset = curr_index;
  else if (curr_index >= offset + vis_height)
    offset = curr_index - vis_height + 1;

  int row = 1;
  for (typename vector<T *>::iterator i = offset; i != list->end() && i < offset + vis_height; ++i, ++row) {
    T *thing = *i;
    wmove(win, row, 1);
    if (thing == curr_item)
      wattron(win, A_REVERSE);
    waddch(win, ' ');
    string list_entry = thing->name();
    make_fit(list_entry, 2);
    waddstr(win, list_entry.c_str());
    waddch(win, ' ');
    if (thing == curr_item)
      wattroff(win, A_REVERSE);
  }
}

#ifdef DEBUG

template <typename T>
void ListWindow<T>::debug() {
  fprintf(stderr, "list_window %p, offset %p\n", this, (void *)&offset);
  Window::debug();
  fprintf(stderr, "  list in list window %p:\n", this);
  fprintf(stderr, "  address of curr_item = %p\r\n", curr_item);
}

#endif
