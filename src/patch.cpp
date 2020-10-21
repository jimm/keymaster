#include <stdlib.h>
#include <set>
#include "patch.h"
#include "output.h"
#include "vector_utils.h"

Patch::Patch(sqlite3_int64 id, const char *patch_name)
  : DBObj(id), Named(patch_name), _running(false),
    _start_message(0), _stop_message(0)
{
}

Patch::~Patch() {
  for (auto& conn : _connections)
    delete conn;
}

void Patch::start() {
  if (_running)
    return;

  send_message_to_outputs(_start_message);
  for (auto& conn : _connections)
    conn->start();
  _running = true;
}

bool Patch::is_running() {
  return _running;
}

void Patch::stop() {
  if (!_running)
    return;

  for (auto& conn : _connections)
    conn->stop();
  send_message_to_outputs(_stop_message);
  _running = false;
}

void Patch::set_start_message(Message *msg) {
  if (_start_message != msg) {
    _start_message = msg;
    changed();
  }
}

void Patch::set_stop_message(Message *msg) {
  if (_stop_message != msg) {
    _stop_message = msg;
    changed();
  }
}

void Patch::add_connection(Connection *conn) {
  conn->add_observer(this);
  _connections.push_back(conn);
  if (is_running())
    conn->start();
  changed();
}

void Patch::remove_connection(Connection *conn) {
  conn->remove_observer(this);
  if (is_running())
    conn->stop();
  erase(_connections, conn);
  delete conn;
  changed();
}

void Patch::send_message_to_outputs(Message *message) {
  if (message == nullptr)
    return;

  PmEvent event = {0, 0};
  set<Output *> outputs;
  for (auto& conn : _connections)
    outputs.insert(conn->output());

  for (auto& out : outputs)
    message->send_to(*out);
}

void Patch::update(Observable *o, void *arg) {
  changed();
}
