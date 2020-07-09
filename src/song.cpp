#include <stdlib.h>
#include <string.h>
#include "song.h"

Song::Song(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name), bpm(120), clock_on_at_start(false)
{
}

Song::~Song() {
  for (auto& patch : patches)
    delete patch;
}
