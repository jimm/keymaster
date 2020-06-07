#ifndef NAMED_H
#define NAMED_H

#include <string>

using namespace std;

class Named {
public:
  string name;

  Named(const char *name);
  virtual ~Named();
};

#endif /* NAMED_H */
