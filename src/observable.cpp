#include "observable.h"

void Observable::add_observer(Observer *o) {
  for (auto observer : observers)
    if (observer == o)
      return;
  observers.push_back(o);
}

void Observable::remove_observer(Observer *o) {
  for (vector<Observer *>::iterator i = observers.begin(); i != observers.end(); ++i) {
    if (*i == o) {
      observers.erase(i);
      return;
    }
  }
}

void Observable::changed(void *arg) {
  for (auto &observer : observers)
    observer->update(this, arg);
}
