#ifndef CLOCK_H
#define CLOCK_H

#include <vector>
#include <pthread.h>
#include "clock_monitor.h"

class Input;

class Clock {
public:
  std::vector<Input *> &inputs;
  long nanosecs_per_tick;
  int tick_within_beat;
  ClockMonitor *clock_monitor;

  Clock(std::vector<Input *> &inputs);
  ~Clock();

  float bpm() { return _bpm; }
  void set_bpm(float bpm);

  void start();
  void stop();
  long tick();
  bool is_running() { return thread != nullptr; }

  // m may be nullptr
  void set_monitor(ClockMonitor *m) { clock_monitor = m; }

protected:
  float _bpm;
  pthread_t thread;
};

#endif /* CLOCK_H */
