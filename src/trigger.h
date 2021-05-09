#ifndef TRIGGER_H
#define TRIGGER_H

#include <portmidi.h>
#include "db_obj.h"
#include "message.h"
#include "observable.h"

class KeyMaster;

typedef enum TriggerAction {
  TA_NEXT_SONG,
  TA_PREV_SONG,
  TA_NEXT_PATCH,
  TA_PREV_PATCH,
  TA_PANIC,
  TA_SUPER_PANIC,
  TA_TOGGLE_CLOCK,
  TA_MESSAGE
} TriggerAction;

class Input;

class Trigger : public DBObj, public Observable {
public:
  Trigger(sqlite3_int64 id, TriggerAction action, Message *output);
  ~Trigger();

  inline int trigger_key_code() { return _trigger_key_code; }
  inline PmMessage trigger_message() { return _trigger_message; }
  inline TriggerAction action() { return _action; }
  inline Message *output_message() { return _output_message; }

  void set_trigger_key_code(int key_code);
  void set_trigger_message(Input *input, PmMessage message);
  void set_action(TriggerAction action);
  void set_output_message(Message *msg);

  Input *input();               // may return nullptr
  void remove_from_input();     // does nothing if no input

  bool signal_message(PmMessage msg);
  bool signal_key(int key_code);

  string to_list_window_string();

private:
  Input *_trigger_input;
  int _trigger_key_code;
  PmMessage _trigger_message;    // might be all zeroes
  TriggerAction _action;
  Message *_output_message;

  void perform_action();
};

#endif /* TRIGGER_H */