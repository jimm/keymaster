#include "observable.h"

void Observable::add_observer(Observer *o) {
  observers.push_back(o);
}

void Observable::remove_observer(Observer *o) {
  std::remove(observers.begin(), observers.end(), o);
}

void Observable::changed(void *arg) {
  for (auto &observer : observers)
    observer->update(this, arg);
}
