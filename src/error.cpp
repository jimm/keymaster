#include "error.h"
#include "wx/app.h"

void error_message(const char * const msg) {
  fprintf(stderr, "%s\n", msg);
  App *app = app_instance();
  if (app != nullptr) {
    app->show_user_message(msg, 10);
  }
}
