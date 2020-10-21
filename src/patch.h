#ifndef PATCH_H
#define PATCH_H

#include <vector>
#include <portmidi.h>
#include "db_obj.h"
#include "named.h"
#include "connection.h"
#include "input.h"
#include "observer.h"

class Patch : public DBObj, public Named, public Observer {
public:

  Patch(sqlite3_int64 id, const char *name);
  ~Patch();

  void start();
  bool is_running();
  void stop();

  inline vector<Connection *> &connections() { return _connections; }
  inline Message *start_message() { return _start_message; }
  inline Message *stop_message() { return _stop_message; }

  void set_start_message(Message *msg);
  void set_stop_message(Message *msg);

  void add_connection(Connection *conn);
  void remove_connection(Connection *conn);

  void update(Observable *o, void *arg);

private:
  vector<Connection *> _connections;
  Message *_start_message;
  Message *_stop_message;
  bool _running;

  void send_message_to_outputs(Message *message);
};

#endif /* PATCH_H */
