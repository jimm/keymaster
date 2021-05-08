#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <vector>
#include "window.h"

using namespace std;

class InfoWindow : public Window {
public:
  vector<char *> *text_lines;
  vector<char *> *help_lines;
  vector<char *> *contents_lines;
  vector<char *> *display_list;

  InfoWindow(struct rect, const char *);
  ~InfoWindow();

  void set_contents(vector<char *> *text_lines);
  void set_contents(string *text_string);

  void draw();
};

vector<char *> *info_window_text_to_lines(const char *);
void info_window_free_lines(vector<char *> *);

#endif /* INFO_WINDOW_H */
