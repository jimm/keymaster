#ifndef FORMATTER_H
#define FORMATTER_H

#include <portmidi.h>
#include "connection.h"

void note_num_to_name(int num, char *buf);
int note_name_to_num(const char *str); // str may point to an integer string like "64" as well

void format_program(int bank_msb, int bank_lsb, int prog, char *buf);
void format_controllers(Connection *conn, char *buf);

// Translate floating-point value (to a precision of 0.001) to a string,
// removing trailing zeroes and decimal point if possible.
void format_float(float val, char *buf);

// Handles "0x" prefix and negative numbers.
int int_from_chars(const char *str);

// Translates up to first two hex chars into an unsigned char value. Zero,
// one, or two chars used.
unsigned char hex_to_byte(const char *hex);

// Reads two-digit hex chars and converts them to bytes, returning a newly
// allocated buffer. Ignores any non-hex characters in `hex`.
unsigned char * hex_to_bytes(const char *hex);

// Converts `bytes` into two-digit hex characters and returns a newly
// allocated zero-terminated buffer.
char * bytes_to_hex(unsigned char *bytes, int len);

// Translates hex bytes in `str` into a single non-sysex MIDI message. If
// `str` is `nullptr` returns a message consisting of three zero bytes.
PmMessage message_from_bytes(const char *str);

#endif /* FORMATTER_H */
