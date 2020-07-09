#include <sys/time.h>
#include "keymaster.h"
#include "clock.h"

static PmMessage CLOCK_MESSAGE = Pm_Message(CLOCK, 0, 0);

void *clock_send_thread(void *clock_ptr) {
  struct timespec rqtp = {0, 0L};
  Clock *clock = (Clock *)clock_ptr;

  while (clock->is_running()) {
    rqtp.tv_nsec = clock->tick();
    if (nanosleep(&rqtp, nullptr) == -1)
      return nullptr;
  }
  return nullptr;
}

Clock::Clock()
  : thread(nullptr)
{
  set_bpm(120);
}

Clock::~Clock() {
  if (is_running())
    stop();
}

void Clock::set_bpm(float new_val) {
  if (_bpm != new_val) {
    _bpm = new_val;
    nanosecs_per_tick = (long)(2.5e9 / _bpm);
  }
}

void Clock::start() {
  if (is_running())
    return;
  int status = pthread_create(&thread, 0, clock_send_thread, this);
}

void Clock::stop() {
  thread = nullptr;
}

// Sends CLOCK message downstream and returns the amount of time to wait
// until the next tick, in nanoseconds.
long Clock::tick() {
  struct timeval tp;
  struct timezone tzp;

  gettimeofday(&tp, &tzp);
  long start_msecs = (tp.tv_sec * 1000L) + tp.tv_usec;

  KeyMaster *km = KeyMaster_instance();
  if (km != nullptr) {
    for (auto &input : KeyMaster_instance()->inputs)
      input->read(CLOCK_MESSAGE);
  }

  gettimeofday(&tp, &tzp);
  long end_msecs = (tp.tv_sec * 1000L) + tp.tv_usec;

  return nanosecs_per_tick - (end_msecs - start_msecs);
}
