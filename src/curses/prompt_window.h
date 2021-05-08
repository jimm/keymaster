#ifndef PROMPT_WINDOW_H
#define PROMPT_WINDOW_H

#include "window.h"

class PromptWindow : public Window {
public:
  PromptWindow(const char *title);
  ~PromptWindow();

  string gets();

private:
  void draw();
  string read_string();
};

#endif /* PROMPT_WINDOW_H */
