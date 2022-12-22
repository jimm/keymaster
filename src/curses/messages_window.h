#ifndef MESSAGES_WINDOW_H
#define MESSAGES_WINDOW_H

#include "window.h"
#include "../message.h"

class MessagesWindow : public Window {
public:
  MessagesWindow(struct rect, const char *);
  ~MessagesWindow();

  void draw();
};

#endif /* MESSAGES_WINDOW_H */
