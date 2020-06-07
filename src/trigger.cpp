#include <stdlib.h>
#include <string.h>
#include "trigger.h"
#include "keymaster.h"

Trigger::Trigger(sqlite3_int64 id, TriggerAction ta, Message *out_msg)
  : DBObj(id),
    trigger_key_code(UNDEFINED), trigger_message(Pm_Message(0, 0, 0)),
    action(ta), output_message(out_msg), trigger_input(nullptr)
{
}

Trigger::~Trigger() {
}

void Trigger::set_trigger_key_code(int key_code) {
  trigger_key_code = key_code;
}

void Trigger::set_trigger_message(Input *input, PmMessage message) {
  remove_from_input();
  trigger_message = message;
  input->add_trigger(this);
  trigger_input = input;
}

Input *Trigger::input() {
  return trigger_input;
}

bool Trigger::signal_message(PmMessage msg) {
  if (msg == trigger_message) {
      perform_action();
      return true;
  }
  return false;
}

bool Trigger::signal_key(int key_code) {
  if (key_code == trigger_key_code) {
      perform_action();
      return true;
  }
  return false;
}

void Trigger::perform_action() {
  KeyMaster *km = KeyMaster_instance();

  switch (action) {
  case TA_NEXT_SONG:
    km->next_song();
    break;
  case TA_PREV_SONG:
    km->prev_song();
    break;
  case TA_NEXT_PATCH:
    km->next_patch();
    break;
  case TA_PREV_PATCH:
    km->prev_patch();
    break;
  case TA_PANIC:
    km->panic(false);
    break;
  case TA_SUPER_PANIC:
    km->panic(true);
    break;
  case TA_MESSAGE:
    output_message->send();
    break;
  }
}

void Trigger::remove_from_input() {
  if (trigger_input != nullptr)
    trigger_input->remove_trigger(this);
  trigger_input = nullptr;
}
