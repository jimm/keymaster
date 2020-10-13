#include <stdlib.h>
#include <set>
#include "patch.h"
#include "output.h"
#include "vector_utils.h"

Patch::Patch(sqlite3_int64 id, const char *patch_name)
  : DBObj(id), Named(patch_name), running(false),
    start_message(0), stop_message(0)
{
}

Patch::~Patch() {
  for (auto& conn : connections)
    delete conn;
}

void Patch::start() {
  if (running)
    return;

  send_message_to_outputs(start_message);
  for (auto& conn : connections)
    conn->start();
  running = true;
}

bool Patch::is_running() {
  return running;
}

void Patch::stop() {
  if (!running)
    return;

  for (auto& conn : connections)
    conn->stop();
  send_message_to_outputs(stop_message);
  running = false;
}

void Patch::add_connection(Connection *conn) {
  connections.push_back(conn);
  if (is_running())
    conn->start();
}

// Caller must destroy the connection.
void Patch::remove_connection(Connection *conn) {
  if (is_running())
    conn->stop();
  erase(connections, conn);
}

void Patch::send_message_to_outputs(Message *message) {
  if (message == nullptr)
    return;

  PmEvent event = {0, 0};
  set<Output *> outputs;
  for (auto& conn : connections)
    outputs.insert(conn->output);

  for (auto& out : outputs)
    message->send_to(*out);
}
