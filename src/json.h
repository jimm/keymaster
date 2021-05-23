#ifndef JSON_H
#define JSON_H

#include <sstream>
#include <vector>
#include <sqlite3.h>

using namespace std;

class KeyMaster;
class Message;
class Trigger;
class Song;
class Patch;
class Connection;
class SetList;

class JSON {
public:
  JSON &encode(string str);
  JSON &encode(sqlite3_int64 id);
  JSON &encode(KeyMaster &obj); // status
  JSON &encode(Message &obj);
  JSON &encode(Trigger &obj);
  JSON &encode(Song &obj);
  JSON &encode(Patch &obj);
  JSON &encode(Connection &obj);
  JSON &encode(SetList &obj);

  JSON &encode(vector<string> v);
  JSON &encode(vector<sqlite3_int64> ids);
  // JSON &encode(vector<Message *> v);
  // JSON &encode(vector<Trigger *> v);
  // JSON &encode(vector<Song *> v);
  // JSON &encode(vector<Patch *> v);
  // JSON &encode(vector<Connection *> v);
  // JSON &encode(vector<SetList *> v);

  string str() { return _ostr.str(); }

private:
  ostringstream _ostr;

  JSON &field_name(string str);
  JSON &quote(string s);
  JSON &quote(const char *s);
};

#endif /* JSON_H */
