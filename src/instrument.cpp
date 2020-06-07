#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "instrument.h"

Instrument::Instrument(sqlite3_int64 id, const char *name, const char *pname,
                       int portmidi_port_num)
  : DBObj(id), Named(name),
    port_name(pname), midi_monitor(nullptr), enabled(false)
{
  port_num = portmidi_port_num;
  num_io_messages = 0;
}

bool Instrument::real_port() {
  return port_num != pmNoDevice;
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
    sprintf(buf, "error closing instrument %s: %s\n", name.c_str(),
            Pm_GetErrorText(err));
    error_message(buf);
  }
}

// only used during testing
void Instrument::clear() {
  num_io_messages = 0;
}
