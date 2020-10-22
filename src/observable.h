#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <set>
#include "observer.h"

using namespace std;

class Observable {
public:
  virtual ~Observable();

  virtual void add_observer(Observer *o);
  virtual void remove_observer(Observer *o);

  virtual void changed(void *arg = nullptr);

private:
  set<Observer *> observers;
};

#endif /* OBSERVABLE_H */
