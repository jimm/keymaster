#include "observable.h"
#include "vector_utils.h"

Observable::~Observable() {
  observers.clear();
}

void Observable::add_observer(Observer *o) {
  observers.insert(o);
}

void Observable::remove_observer(Observer *o) {
  observers.erase(o);
}

void Observable::changed(void *arg) {
  for (auto &observer : observers)
    observer->update(this, arg);
}
