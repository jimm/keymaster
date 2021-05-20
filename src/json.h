#ifndef JSON_H
#define JSON_H

#include <sstream>
#include <vector>

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
  void encode(KeyMaster &obj); // status
  void encode(Message &obj);
  void encode(Trigger &obj);
  void encode(Song &obj);
  void encode(Patch &obj);
  void encode(Connection &obj);
  void encode(SetList &obj);

  void encode(vector<Message *> &v);
  void encode(vector<Trigger *> &v);
  void encode(vector<Song *> &v);
  void encode(vector<Patch *> &v);
  void encode(vector<Connection *> &v);
  void encode(vector<SetList *> &v);

  void encode(string name, string val);
  void encode(string name, int val);

  string str() { return _ostr.str(); }

private:
  ostringstream _ostr;

  void quote(string &s);
  void quote(const char *s);
};

#endif /* JSON_H */