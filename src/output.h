#ifndef OUTPUT_H
#define OUTPUT_H

#include <portmidi.h>
#include "instrument.h"
#include "input.h"

class Output : public Instrument {
public:
  Output(int id, const char *name, const char *port_name, int port_num);

  void write(PmEvent *buf, int len);

protected:
  virtual bool start_midi();
};

#endif /* OUTPUT_H */
