#include "observable.h"
#include "vector_utils.h"

void Observable::add_observer(Observer *o) {
  for (auto observer : observers)
    if (observer == o)
      return;
  observers.push_back(o);
}

void Observable::remove_observer(Observer *o) {
  erase(observers, o);
}

void Observable::changed(void *arg) {
  for (auto &observer : observers)
    observer->update(this, arg);
}
