#ifndef WEB_H
#define WEB_H

#include <sstream>
#include <map>
#include <stdio.h>
#include "keymaster.h"
#include "named.h"

using namespace std;

class Web {
public:
  map<string, string> params;   // only public for testing purposes

  Web(KeyMaster *km, int port_num);
  ~Web();

  int run();

  // only public for testing purposes
  string status_json();
  void parse_params(const char *uri, int path_len);

private:
  KeyMaster *km;
  int port_num;
  FILE *stream;
  int childfd;

  void error(const char *);
  void cerror(const char *cause, const char *error_number,
              const char *shortmsg, const char *longmsg);
  void return_status();
  void append_connection(ostringstream &, Connection *);
  void append_instrument_fields(ostringstream &, Instrument *);
  string unencode(const char *p);
};

#endif /* WEB_H */
