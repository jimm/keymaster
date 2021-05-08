#ifndef WEB_H
#define WEB_H

#include <stdio.h>
#include <string>
#include "keymaster.h"
#include "named.h"

using namespace std;

class Web {
public:
  Web(KeyMaster *km, int port_num);
  ~Web();

  int run();

private:
  KeyMaster *km;
  int port_num;
  FILE *stream;
  int childfd;

  void error(const char *);
  void cerror(const char *cause, const char *error_number,
              const char *shortmsg, const char *longmsg);
  void return_status();
  void append_connection(string &, Connection *);
  void append_instrument_fields(string &, Instrument *);
};

#endif /* WEB_H */
