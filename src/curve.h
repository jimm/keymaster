#ifndef CURVES_H
#define CURVES_H

#include <vector>
#include <string>
#include "db_obj.h"
#include "named.h"

class KeyMaster;

class Curve : public DBObj, public Named {
public:
  string _short_name;
  unsigned char curve[128];

  Curve(sqlite3_int64 id, const char *c_name, const char *c_short_name);
  Curve(const Curve &other);

  void from_chars(const char *str);

  string short_name() { return _short_name; }
};

extern void generate_default_curves(vector<Curve *> &vec);

#endif /* CURVES_H */
