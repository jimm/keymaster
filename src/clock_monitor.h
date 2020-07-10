#ifndef CLOCK_MONITOR_H
#define CLOCK_MONITOR_H

class Input;
class Output;

// An abstract class that defines two public methods for monitoring clock
// state changes.
class ClockMonitor {
public:
  virtual ~ClockMonitor() {}

  virtual void monitor_bpm(int bpm) {}
  virtual void monitor_start() {}
  virtual void monitor_stop() {}
  virtual void monitor_beat() {}
};

#endif /* CLOCK_MONITOR_H */
