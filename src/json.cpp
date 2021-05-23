#include <iomanip>
#include <sqlite3.h>
#include "json.h"
#include "keymaster.h"
#include "cursor.h"

// ================ template funcs ================

template <class T>
vector<string> names(vector<T *> &list) {
  vector<string> names;
  for (auto& named : list)
    names.push_back(named->name());
  return names;
}

template <class T>
vector<sqlite3_int64> ids(vector<T *> &list) {
  vector<sqlite3_int64> ids;
  for (auto& db_obj : list)
    ids.push_back(db_obj->id());
  return ids;
}

// ================ JSON funcs ================

JSON &JSON::encode(string str) {
  return quote(str);
}

JSON &JSON::encode(sqlite3_int64 id) {
  _ostr << id;
  return *this;
}

// KeyMaster status
JSON &JSON::encode(KeyMaster &obj) {
  _ostr << '{';
  field_name("set_lists");
  encode(names(obj.set_lists()));
  _ostr << ',';

  SetList *set_list = obj.cursor()->set_list();
  field_name("set_list");
  encode(names(set_list->songs()));
  _ostr << ',';

  // FIXME
  // append_name_list(_ostr, "songs", **************** FIXME ****************

  _ostr << '}';
  return *this;
}

JSON &JSON::encode(Message &obj) {
  _ostr << '{';
  field_name("name");
  quote(obj.name());
  _ostr << ',';
  field_name("bytes");
  quote(obj.to_string());
  _ostr << '}';
  return *this;
}

JSON &JSON::encode(Trigger &obj) {
  _ostr << '{';
  field_name("key_code");
  if (obj.trigger_key_code() == UNDEFINED)
    _ostr << "null";
  else
    _ostr << obj.trigger_key_code();
  _ostr << ',';

  if (obj.input()) {
    field_name("input_id");
    encode(obj.input()->id());
    _ostr << ',';
    field_name("trigger_message");
    _ostr << '"' << std::setfill('0') << std::setw(8) << std::hex << obj.trigger_message() << '"';
  }
  else {
    field_name("input_id");
    _ostr << "null,";
    field_name("trigger_message");
    _ostr << "null";
  }
  _ostr << ',';

  field_name("action");
  encode(obj.action_string());
  _ostr << ',';

  field_name("message_id");
  if (obj.output_message())
    _ostr <<  obj.output_message()->id();
  else
    _ostr << "null";

  _ostr << '}';
  return *this;
}

JSON &JSON::encode(Song &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("patch_ids");
  encode(ids(obj.patches()));
  _ostr << '}';
  return *this;
}

JSON &JSON::encode(Patch &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("connection_ids");
  encode(ids(obj.connections()));
  _ostr << '}';
  return *this;
}

JSON &JSON::encode(Connection &obj) {
  _ostr << "{";

  field_name("input");
  encode(obj.input()->name());
  _ostr << ',';

  field_name("input_chan");
  if (obj.input_chan() == -1)
    encode("all");
  else
    _ostr << (obj.input_chan() + 1);
  _ostr << ',';

  field_name("output");
  encode(obj.output()->name());
  _ostr << ',';

  field_name("output_chan");
  if (obj.output_chan() == -1)
    encode("all");
  else
    _ostr << (obj.output_chan() + 1);
  _ostr << ',';

  field_name("prog");
  _ostr << '{';

  field_name("bank_msb");
  _ostr << obj.program_bank_msb();
  _ostr << ',';

  field_name("bank_lsb");
  _ostr << obj.program_bank_lsb();
  _ostr << ',';

  field_name("pc");
  _ostr << obj.program_prog();
  _ostr << "},";

  field_name("zone");
  _ostr << '{';

  field_name("low");
  _ostr << obj.zone_low();
  _ostr << ',';

  field_name("high");
  _ostr << obj.zone_low();
  _ostr << "},";

  field_name("xpose");
  _ostr << obj.xpose();

  // FIXME: curve, filter, bools, CC map

  _ostr << '}';
  return *this;
}

JSON &JSON::encode(SetList &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("song_ids");
  encode(ids(obj.songs()));
  _ostr << '}';
  return *this;
}

// ================ vectors of things ================

JSON &JSON::encode(vector<string> v) {
  _ostr << '[';
  for (auto s : v) {
    if (s != v.front())
      _ostr << ',';
    quote(s);
  }
  _ostr << ']';
  return *this;
}

JSON &JSON::encode(vector<sqlite3_int64> ids) {
  _ostr << '[';
  for (auto id : ids) {
    if (id != ids.front())
      _ostr << ',';
    _ostr << id;
  }
  _ostr << ']';
  return *this;
}

// JSON &JSON::encode(vector<Message *> v) {
//   return *this;
// }

// JSON &JSON::encode(vector<Trigger *> v) {
//   return *this;
// }

// JSON &JSON::encode(vector<Song *> v) {
//   return *this;
// }

// JSON &JSON::encode(vector<Patch *> v) {
//   return *this;
// }

// JSON &JSON::encode(vector<Connection *> v) {
//   return *this;
// }

// JSON &JSON::encode(vector<SetList *> v) {
//   append_name_list(_ostr, "set_lists", v);
//   return *this;
// }

// ================ helpers ================

JSON &JSON::quote(string s) {
  return quote(s.c_str());
}

JSON &JSON::quote(const char *s) {
  _ostr << '"';
  while (*s) {
    switch (*s) {
    case '"':
      _ostr << '\\' << *s;
      break;
    case '\n':
      _ostr << "\\n";
      break;
    default:
      _ostr << *s;
      break;
    }
    ++s;
  }
  _ostr << '"';
  return *this;
}

JSON &JSON::field_name(string str) {
  quote(str);
  _ostr << ':';
  return *this;
}
