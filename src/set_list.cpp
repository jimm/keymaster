#include <stdlib.h>
#include <string.h>
#include "set_list.h"

SetList::SetList(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name)
{
}

SetList::~SetList() {
}
