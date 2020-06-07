#ifndef PATCH_H
#define PATCH_H

#include <vector>
#include <portmidi.h>
#include "db_obj.h"
#include "named.h"
#include "connection.h"
#include "input.h"

class Patch : public DBObj, public Named {
public:
  vector<Connection *> connections;
  Message *start_message;
  Message *stop_message;
  bool running;

  Patch(sqlite3_int64 id, const char *name);
  ~Patch();

  void start();
  bool is_running();
  void stop();

private:
  void send_message_to_outputs(Message *message);
};

#endif /* PATCH_H */
