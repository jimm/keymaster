#ifndef NAMED_H
#define NAMED_H

#include <string>
#include "observable.h"

using namespace std;

class Named : public Observable {
public:

  Named(const char *name);
  virtual ~Named();

  inline string &name() { return _name; }
  void set_name(const char *name);
  void set_name(string &name) { set_name(name.c_str()); }

private:
  string _name;
};

#endif /* NAMED_H */
