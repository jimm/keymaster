#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <vector>
#include "observer.h"

using namespace std;

class Observable {
public:
  virtual void add_observer(Observer *o);
  virtual void remove_observer(Observer *o);

  virtual void changed(void *arg = nullptr);

private:
  vector<Observer *> observers;
};

#endif /* OBSERVABLE_H */
