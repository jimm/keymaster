#include <stdlib.h>
#include <string.h>
#include "trigger.h"
#include "keymaster.h"

Trigger::Trigger(sqlite3_int64 id, TriggerAction ta, Message *out_msg)
  : DBObj(id),
    _trigger_key_code(UNDEFINED), _trigger_message(Pm_Message(0, 0, 0)),
    _action(ta), _output_message(out_msg), _trigger_input(nullptr)
{
}

Trigger::~Trigger() {
  remove_from_input();
}

void Trigger::set_trigger_key_code(int key_code) {
  if (_trigger_key_code != key_code) {
    _trigger_key_code = key_code;
    changed();
  }
}

// `message` might be all zeroes, which will never match an incoming MIDI
// message.
void Trigger::set_trigger_message(Input *input, PmMessage message) {
  if (_trigger_input == input && _trigger_message == message)
    return;

  remove_from_input();
  _trigger_message = message;
  input->add_trigger(this);
  _trigger_input = input;
  changed();
}

void Trigger::set_action(TriggerAction action) {
  if (_action != action) {
    _action = action;
    changed();
  }
}

void Trigger::set_output_message(Message *msg) {
  if (_output_message != msg) {
    _output_message = msg;
    changed();
  }
}

Input *Trigger::input() {
  return _trigger_input;
}

bool Trigger::signal_message(PmMessage msg) {
  if (msg == _trigger_message) {
      perform_action();
      return true;
  }
  return false;
}

bool Trigger::signal_key(int key_code) {
  if (key_code == _trigger_key_code) {
      perform_action();
      return true;
  }
  return false;
}

void Trigger::perform_action() {
  KeyMaster *km = KeyMaster_instance();

  switch (_action) {
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
  case TA_TOGGLE_CLOCK:
    km->toggle_clock();
    break;
  case TA_MESSAGE:
    _output_message->send_to_all_outputs();
    break;
  }
}

void Trigger::remove_from_input() {
  if (_trigger_input != nullptr)
    _trigger_input->remove_trigger(this);
  _trigger_input = nullptr;
}
