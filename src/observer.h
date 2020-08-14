#ifndef OBSERVER_H
#define OBSERVER_H

class Observable;

class Observer {
public:
  virtual void update(Observable *o, void *arg) {}
};

#endif /* OBSERVER_H */
