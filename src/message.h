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

  Message(sqlite3_int64 id, const char *name);

  void from_string(string &str) { from_chars(str.c_str()); }
  void from_chars(const char *str);

  void from_editable_string(const string &str);

  string to_string();
  string to_editable_string();

  void send_to_all_outputs();
  void send_to(Output &);

  // Public ONLY for testing; nobody else needs to see the raw events.
  inline vector<PmEvent> &events() { return _events; }

private:
  vector<PmEvent> _events;
};

#endif /* MESSAGE_H */
