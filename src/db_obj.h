#ifndef DB_OBJ_H
#define DB_OBJ_H

#include <sqlite3.h>

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