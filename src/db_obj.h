#ifndef DB_OBJ_H
#define DB_OBJ_H

#include <sqlite3.h>

// Storage/accessors for database ids. These ids are only used during
// loading and saving of data by a Storage object. Do NOT rely on their
// values to persist across saves.
class DBObj {
public:
  DBObj(sqlite3_int64 i) : _id(i) {}
  virtual ~DBObj() {}

  sqlite3_int64 id() { return _id; }
  void set_id(sqlite3_int64 id) { _id = id; }

private:
  sqlite3_int64 _id;
};

#endif /* DB_OBJ_H */