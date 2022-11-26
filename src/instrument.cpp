#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "instrument.h"

Instrument::Instrument(sqlite3_int64 id, PmDeviceID dev_id, const char *dev_device_name, const char *c_name)
  : DBObj(id), Named(c_name == nullptr ? dev_device_name : c_name),
    device_id(dev_id), device_name(dev_device_name), enabled(false)
{
  num_io_messages = 0;
}

bool Instrument::real_port() {
  return device_id != pmNoDevice;
}

void Instrument::start() {
  if (!real_port()) {
    enabled = false;
    return;
  }
  enabled = start_midi();
}

void Instrument::stop() {
  if (real_port() && enabled)
    stop_midi();
  enabled = false;
}

void Instrument::stop_midi() {
  PmError err = Pm_Close(stream);
  if (err != 0) {
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "error closing instrument %s: %s\n", name().c_str(),
            Pm_GetErrorText(err));
    error_message(buf);
  }
}

// only used during testing
void Instrument::clear() {
  num_io_messages = 0;
}
