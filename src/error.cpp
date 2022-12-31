#include <iostream>
#include <cstdarg>
#include "error.h"
#include "wx/app.h"

void error_message(const char *fmt...) {
  char buf[BUFSIZ];

  va_list args;
  va_start(args, fmt);

  vsnprintf(buf, BUFSIZ, fmt, args);

  fprintf(stderr, "%s\n", buf);
  App *app = app_instance();
  if (app != nullptr) {
    app->show_user_message(buf, 10);
  }
}
