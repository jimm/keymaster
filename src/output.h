#ifndef OUTPUT_H
#define OUTPUT_H

#include "instrument.h"
#include "input.h"

class Output : public Instrument {
public:
  Output(sqlite3_int64 id, PmDeviceID device_id, const char *device_name, const char *name = nullptr);

  void write(PmEvent *buf, int len);

protected:
  virtual bool start_midi();
};

#endif /* OUTPUT_H */
