#include "json.h"
#include "keymaster.h"
#include "cursor.h"

void stream_quote(ostringstream &ostr, const char *quote_me) {
  ostr << '"';
  while (*quote_me) {
    switch (*quote_me) {
    case '"':
      ostr << '\\' << *quote_me;
      break;
    case '\n':
      ostr << "\\n";
      break;
    default:
      ostr << *quote_me;
      break;
    }
    ++quote_me;
  }
  ostr << '"';
}

template <class T>
void append_name_list(ostringstream &ostr, const char *name, vector<T *> &list) {
  stream_quote(ostr, name);
  ostr << ":[";
  for (auto& named : list) {
    if (named != list.front())
      ostr << ',';
    stream_quote(ostr, named->name().c_str());
  }
  ostr << ']';
}

template <class T>
void append_id_list(ostringstream &ostr, const char *name, vector<T *> &list) {
  ostr << '[';
  for (auto& db_obj : list) {
    if (db_obj != list.front())
      ostr << ',';
    ostr << db_obj->id();
  }
  ostr << ']';
}

// KeyMaster status
void JSON::encode(KeyMaster &obj) {
  _ostr << '{';
  append_name_list(_ostr, "set_lists", obj.set_lists());
  _ostr << ',';

  SetList *set_list = obj.cursor()->set_list();
  encode("set_list", set_list->name());
  _ostr << ',';
  // FIXME
  // append_name_list(_ostr, "songs", **************** FIXME ****************
  _ostr << '}';
}

void JSON::encode(Message &obj) {
  _ostr << '{';
  encode("name", obj.name());
  _ostr << ',';
  quote("bytes");
  _ostr << ":\"" << obj.to_editable_string() << "\"}";
}

void JSON::encode(Trigger &obj) {
  _ostr << '{';
  encode("key_code", obj.trigger_key_code());
  _ostr << ',';
  encode("trigger_message_id", obj.trigger_message());
  _ostr << ',';
  encode("action", obj.action());
  _ostr << ',';
  encode("message_id", obj.output_message() ? obj.output_message()->id() : 0);
  _ostr << '}';
}

void JSON::encode(Song &obj) {
  _ostr << '{';
  encode("name", obj.name());
  _ostr << ',';
  append_id_list(_ostr, "patch_ids", obj.patches());
  _ostr << '}';
}

void JSON::encode(Patch &obj) {
  _ostr << '{';
  encode("name", obj.name());
  _ostr << ',';
  append_id_list(_ostr, "connection_ids", obj.connections());
  _ostr << '}';
}

void JSON::encode(Connection &obj) {
  _ostr << "{";

  encode("input", obj.input()->name());
  _ostr << ',';
  if (obj.input_chan() == -1)
    encode("input_chan", "all");
  else
    encode("input_chan", obj.input_chan() + 1);

  _ostr << ',';
  encode("output", obj.output()->name());
  _ostr << ',';
  if (obj.output_chan() == -1)
    encode("output_chan", "all");
  else
    encode("output_chan", obj.output_chan() + 1);

  _ostr << ",\"prog\":{";
  encode("bank_msb", obj.program_bank_msb());
  _ostr << ',';
  encode("bank_lsb", obj.program_bank_lsb());
  _ostr << ',';
  encode("pc", obj.program_prog());
  _ostr << "},\"zone\"{";
  encode("low", obj.zone_low());
  _ostr << ',';
  encode("high", obj.zone_low());
  _ostr << "},";
  encode("xpose", obj.xpose());

  // FIXME: curve, filter, bools, CC map

  _ostr << '}';
}

void JSON::encode(SetList &obj) {
  _ostr << '{';
  encode("name", obj.name());
  _ostr << ',';
  append_id_list(_ostr, "song_ids", obj.songs());
  _ostr << '}';
}

// ================ vectors of things ================

void JSON::encode(vector<Message *> &v) {
}

void JSON::encode(vector<Trigger *> &v) {
}

void JSON::encode(vector<Song *> &v) {
}

void JSON::encode(vector<Patch *> &v) {
}

void JSON::encode(vector<Connection *> &v) {
}

void JSON::encode(vector<SetList *> &v) {
  append_name_list(_ostr, "set_lists", v);
}

// ================ name/value pairs ================

void JSON::encode(string name, string val) {
  quote(name);
  _ostr << ':';
  quote(val);
}

void JSON::encode(string name, int val) {
  quote(name);
  _ostr << ':' << val;
}

// ================ helpers ================

void JSON::quote(string &s) {
  stream_quote(_ostr, s.c_str());
}

void JSON::quote(const char *s) {
  stream_quote(_ostr, s);
}
