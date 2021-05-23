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

void JSON::field_name(string str) {
  encode(str);
  _ostr << ':';
}

void JSON::encode(string str) {
  quote(str);
}

// KeyMaster status
void JSON::encode(KeyMaster &obj) {
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
}

void JSON::encode(Message &obj) {
  _ostr << '{';
  field_name("name");
  quote(obj.name());
  _ostr << ',';
  field_name("bytes");
  quote(obj.to_editable_string());
  _ostr << '}';
}

void JSON::encode(Trigger &obj) {
  _ostr << '{';
  field_name("key_code");
  _ostr << obj.trigger_key_code();
  _ostr << ',';
  field_name("trigger_message");
  _ostr << obj.trigger_message();
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
}

void JSON::encode(Song &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("patch_ids");
  encode(ids(obj.patches()));
  _ostr << '}';
}

void JSON::encode(Patch &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("connection_ids");
  encode(ids(obj.connections()));
  _ostr << '}';
}

void JSON::encode(Connection &obj) {
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
}

void JSON::encode(SetList &obj) {
  _ostr << '{';
  field_name("name");
  encode(obj.name());
  _ostr << ',';
  field_name("song_ids");
  encode(ids(obj.songs()));
  _ostr << '}';
}

// ================ vectors of things ================

void JSON::encode(vector<string> v) {
  _ostr << '[';
  for (auto s : v) {
    if (s != v.front())
      _ostr << ',';
    quote(s);
  }
  _ostr << ']';
}

void JSON::encode(vector<sqlite3_int64> ids) {
  _ostr << '[';
  for (auto id : ids) {
    if (id != ids.front())
      _ostr << ',';
    _ostr << id;
  }
  _ostr << ']';
}

// void JSON::encode(vector<Message *> v) {
// }

// void JSON::encode(vector<Trigger *> v) {
// }

// void JSON::encode(vector<Song *> v) {
// }

// void JSON::encode(vector<Patch *> v) {
// }

// void JSON::encode(vector<Connection *> v) {
// }

// void JSON::encode(vector<SetList *> v) {
//   append_name_list(_ostr, "set_lists", v);
// }

// ================ helpers ================

void JSON::quote(string s) {
  quote(s.c_str());
}

void JSON::quote(const char *s) {
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
}
