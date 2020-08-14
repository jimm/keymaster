#ifndef CLOCK_H
#define CLOCK_H

#include <vector>
#include <pthread.h>
#include "observable.h"

class Input;

enum ClockChange {
  ClockChangeBpm,
  ClockChangeStart,
  ClockChangeStop,
  ClockChangeBeat
};

class Clock : public Observable {
public:
  std::vector<Input *> &inputs;
  long nanosecs_per_tick;
  int tick_within_beat;

  Clock(std::vector<Input *> &inputs);
  ~Clock();

  float bpm() { return _bpm; }
  void set_bpm(float bpm);

  void start();
  void stop();
  long tick();
  bool is_running() { return thread != nullptr; }

protected:
  float _bpm;
  pthread_t thread;
};

#endif /* CLOCK_H */
