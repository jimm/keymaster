#ifndef FORMATTER_H
#define FORMATTER_H

#include <portmidi.h>
#include "connection.h"

void note_num_to_name(int num, char *buf);
int note_name_to_num(const char *str); // str may point to an integer string like "64" as well
void format_program(program prog, char *buf);
void format_controllers(Connection *conn, char *buf);
string byte_to_hex(unsigned char byte);
unsigned char hex_to_byte(const char *hex);
int int_from_chars(const char *str);
unsigned char byte_from_chars(const char * const str); // 0 if illegal value
PmMessage message_from_bytes(const char *str);

#endif /* FORMATTER_H */
