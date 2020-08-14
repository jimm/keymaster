#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <portmidi.h>
#include "db_obj.h"
#include "named.h"
#include "observable.h"

#define MIDI_BUFSIZ 128

class Instrument : public DBObj, public Named, public Observable {
public:
  PmDeviceID device_id;
  string device_name;
  PortMidiStream *stream;
  bool enabled;

  PmMessage io_messages[MIDI_BUFSIZ]; // testing only
  int num_io_messages;                // ditto

  Instrument(sqlite3_int64 id, PmDeviceID device_id, const char *device_name,
             const char *name = nullptr);
  virtual ~Instrument() {}

  virtual bool is_input() { return false; }
  virtual bool is_output() { return false; }

  virtual void start();
  virtual void stop();
  bool real_port();

  void clear();                 // testing only

protected:
  virtual bool start_midi() { return false; }
  virtual void stop_midi();
};

#endif /* INSTRUMENT_H */
