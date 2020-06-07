#include <stdlib.h>
#include <string.h>
#include "song.h"

Song::Song(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name)
{
}

Song::~Song() {
  for (auto& patch : patches)
    delete patch;
}
