#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "formatter.h"
#include "consts.h"
#include "connection.h"
#include "error.h"

static const char * NOTE_NAMES[] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};
static const int NOTE_OFFSETS[] = {
  9, 11, 0, 2, 4, 5, 7
};
static const char HEX_DIGITS[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void note_num_to_name(int num, char *buf) {
  int oct = (num / 12) - 1;
  const char *note = NOTE_NAMES[num % 12];
  snprintf(buf, 8, "%s%d", note, oct);
}

// str may point to an integer string like "64" as well
int note_name_to_num(const char *str) {
  char ch = str[0];

  if (isdigit(ch))
    return atoi(str);

  ch = tolower(ch);
  if (ch < 'a' || ch > 'g')
    return 0;

  int from_c = NOTE_OFFSETS[ch - 'a'];
  const char *num_start = str+1;
  int accidental = 0;
  switch (tolower(*num_start)) {
  case 's': case '#':
    accidental = 1;
    ++num_start;
    break;
  case 'f': case 'b':
    accidental = -1;
    ++num_start;
    break;
  }

  int octave = (atoi(num_start) + 1) * 12;
  return octave + from_c + accidental;
}

void format_program(int bank_msb, int bank_lsb, int prog, char *buf) {
  int has_msb = bank_msb != UNDEFINED;
  int has_lsb = bank_lsb != UNDEFINED;
  int has_bank = has_msb || has_lsb;

  snprintf(buf, 3, " %c", has_bank ? '[' : ' ');
  buf += 2;

  if (has_msb)
    snprintf(buf, 4, "%3d", bank_msb);
  else
    strcat(buf, "   ");
  buf += 3;

  snprintf(buf, 3, "%c ", has_bank ? ',' : ' ');
  buf += 2;

  if (has_lsb)
    snprintf(buf, 4, "%3d", bank_lsb);
  else
    strcat(buf, "   ");
  buf += 3;

  snprintf(buf, 3, "%c ", has_bank ? ']' : ' ');
  buf += 2;

  if (prog != UNDEFINED)
    snprintf(buf, 5, " %3d", prog);
  else
    strcat(buf, "    ");
}

void format_controllers(Connection *conn, char *buf) {
  int first = true;

  strcat(buf, " ");
  buf += 1;
  for (int i = 0; i < 128; ++i) {
    Controller *cc = conn->cc_map(i);
    if (cc == nullptr)
      continue;

    if (first) first = false; else { strcat(buf, ", "); buf += 2; }
    snprintf(buf, 4, "%d", cc->cc_num());
    buf += strlen(buf);

    if (cc->filtered()) {
      *buf++ = 'x';
      continue;
    }

    if (cc->cc_num() != cc->translated_cc_num()) {
      snprintf(buf, 6, "->%d", cc->translated_cc_num());
      buf += strlen(buf);
    }

    if (cc->min_in() != 0 || cc->max_in() != 127
        || cc->min_out() != 0 || cc->max_out() != 127)
    {
      *buf++ = ' ';
      if (cc->pass_through_0())
        *buf++ = '0';
      snprintf(buf, 28, " [%d, %d] -> [%d, %d]",
              cc->min_in(), cc->max_in(),
              cc->min_out(), cc->max_out());
      buf += strlen(buf);
      if (cc->pass_through_127()) {
        snprintf(buf, 4, "127");
        buf += 3;
      }
    }
  }
  *buf = 0;
}

// Translate floating-point value (to a precision of 0.001) to a string,
// removing trailing zeroes and decimal point if possible.
void format_float(float val, char *buf) {
  snprintf(buf, 16, "%0.2f", val);
  char *p = buf + strlen(buf) - 1;
  while (*p == '0') --p;
  if (*p != '.') ++p;
  *p = 0;
}

// Handles "0x" prefix and negative numbers.
int int_from_chars(const char *str) {
  str += strspn(str, " \t");
  if (strlen(str) > 2 && strncasecmp(str, "0x", 2) == 0)
    return (int)strtol(str, 0, 16);

  if (str[0] == '-' || isdigit(str[0]))
    return atoi(str);

  return 0;
}

// ================ parsing MIDI messages ================

// Private helper for `hex_to_byte`.
unsigned char hex_digit_from_char(char ch) {
  switch (ch) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return ch - '0';
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    return ch - 'a' + 10;
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    return ch - 'A' + 10;
  default:
    return 0;
  }
}

// Translates up to first two hex chars into an unsigned char value. Zero,
// one, or two chars used.
unsigned char hex_to_byte(const char *hex) {
  unsigned char val = 0;
  if (isxdigit(*hex)) {
    val = hex_digit_from_char(*hex);
    ++hex;
  }
  if (isxdigit(*hex))
    val = (val << 4) + hex_digit_from_char(*hex);

  return val;
}

// private helper for message_from_bytes
//
bool check_byte_value(int val) {
  if (val >= 0 && val <= 255)
    return true;

  error_message("byte value %d is out of range", val);
  return false;
}

// Reads two-digit hex chars and converts them to bytes, returning a newly
// allocated buffer. Ignores any non-hex characters in `hex`.
unsigned char * hex_to_bytes(const char *hex) {
  unsigned char *buf = (unsigned char *)malloc(strlen(hex) / 2);
  int i = 0;

  while (*hex) {
    if (isxdigit(*hex)) {
      buf[i++] = hex_to_byte(hex);
      ++hex;
      if (*hex) ++hex;          // don't skip over \0 if malformed
    }
    else
      ++hex;
  }

  return (unsigned char *)realloc(buf, i);
}

// Converts `bytes` into two-digit hex characters and returns a newly
// allocated zero-terminated buffer.
char * bytes_to_hex(unsigned char *bytes, int len) {
  char *buf = (char *)malloc(len * 2 + 1);
  buf[len * 2] = 0;

  char *p = buf;
  for (int i = 0; i < len; ++i) {
    *p++ = HEX_DIGITS[(bytes[i] >> 4) & 0x0f];
    *p++ = HEX_DIGITS[bytes[i] & 0x0f];
  }
  *p = 0;
  return buf;
}

// private helper for message_from_bytes
//
unsigned char byte_from_chars(const char * const str) {
  int val = int_from_chars(str);
  return check_byte_value(val) ? val : 0;
}

// Translates hex bytes in `str` into a single non-sysex MIDI message. If
// `str` is `nullptr` returns a message consisting of three zero bytes.
PmMessage message_from_bytes(const char *str) {
  if (str == nullptr)
    return Pm_Message(0, 0, 0);

  int bytes[3] = {0, 0, 0};
  int i = 0;

  for (char *word = strtok((char *)str + strspn(str, " \t"), ", ");
       word != nullptr && i < 3;
       word = strtok(nullptr, ", "))
  {
    int val = int_from_chars(word);
    if (!check_byte_value(val))
      return 0;
    bytes[i++] = byte_from_chars(word);
  }

  return Pm_Message(bytes[0], bytes[1], bytes[2]);
}
