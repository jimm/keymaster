#ifndef SET_LIST_H
#define SET_LIST_H

#include <vector>
#include "db_obj.h"
#include "song.h"

using namespace std;

class SetList : public DBObj, public Named {
public:
  vector<Song *> songs;

  SetList(sqlite3_int64 id, const char *name);
  ~SetList();
};

#endif /* SET_LIST_H */
