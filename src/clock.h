#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

class Clock {
public:
  long nanosecs_per_tick;

  Clock();
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
