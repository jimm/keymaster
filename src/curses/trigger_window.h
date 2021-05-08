#ifndef TRIGGER_WINDOW_H
#define TRIGGER_WINDOW_H

#include "window.h"
#include "../trigger.h"

class TriggerWindow : public Window {
public:
  Trigger *trigger;

  TriggerWindow(struct rect, const char *);
  ~TriggerWindow();

  void set_contents(const char *title, Trigger *trigger);

  void draw();
};

#endif /* TRIGGER_WINDOW_H */
