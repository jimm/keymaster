#ifndef SONG_H
#define SONG_H

#include <vector>
#include "db_obj.h"
#include "patch.h"

using namespace std;

class Song : public DBObj, public Named {
public:
  vector<Patch *> patches;
  string notes;
  int bpm;
  bool clock_on_at_start;

  Song(sqlite3_int64 id, const char *name);
  ~Song();
};

#endif /* SONG_H */
