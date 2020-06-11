#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <portmidi.h>
#include "db_obj.h"
#include "named.h"

using namespace std;

class Output;

class Message : public DBObj, public Named {
public:
  vector<PmEvent> events;       // public for testing only

  Message(sqlite3_int64 id, const char *name);

  void from_string(string &str) { from_chars(str.c_str()); }
  void from_chars(const char *str);

  void from_editable_string(const string &str);

  string to_string();
  string to_editable_string();

  void send_to_all_outputs();
  void send_to(Output &);

private:
  void clear_events();
};

#endif /* MESSAGE_H */
