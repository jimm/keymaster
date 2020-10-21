#include <stdlib.h>
#include <string.h>
#include "named.h"

Named::Named(const char *str)
  : _name(str)
{
}

Named::~Named() {
}

void Named::set_name(const char *name) {
  if (_name != name) {
    _name = name;
    changed();
  }
}
