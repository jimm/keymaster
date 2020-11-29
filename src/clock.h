#ifndef CLOCK_H
#define CLOCK_H

#include <vector>
#include <pthread.h>
#include <portmidi.h>
#include "observable.h"

class Input;

enum ClockChange {
  ClockChangeBpm,
  ClockChangeStart,
  ClockChangeContinue,
  ClockChangeStop
};

class Clock : public Observable {
public:
  std::vector<Input *> &inputs;
  unsigned long microsecs_per_tick;

  Clock(std::vector<Input *> &inputs);
  ~Clock();

  float bpm() { return _bpm; }
  void set_bpm(float bpm);

  void start();
  void continue_clock();
  void stop();
  void tick();
  bool is_running() { return thread != nullptr; }

protected:
  float _bpm;
  pthread_t thread;

  void send(PmMessage msg);
  void start_or_continue(PmMessage msg, ClockChange change_type);
};

#endif /* CLOCK_H */
