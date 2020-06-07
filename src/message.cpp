#include <stdlib.h>
#include <string.h>
#include "keymaster.h"
#include "message.h"
#include "output.h"

Message::Message(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name), events(nullptr), num_events(0)
{
}

Message::~Message() {
  if (events != nullptr)
    free(events);
}


void Message::send() {
  for (auto& out : KeyMaster_instance()->outputs)
    send(*out);
}

void Message::send(Output &out) {
  if (events == nullptr)
    convert_messages();
  out.write(events, num_events);
}

void Message::clear_messages() {
  messages.clear();
  free(events);
  events = nullptr;
}

void Message::convert_messages() {
  num_events = messages.size();
  if (num_events == 0)
    return;

  events = (PmEvent *)calloc(num_events, sizeof(PmEvent));
  for (int i = 0; i < num_events; ++i)
    events[i].message = messages[i];
}
