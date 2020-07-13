#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "formatter.h"
#include "consts.h"
#include "connection.h"

static const char * NOTE_NAMES[] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};
static const int NOTE_OFFSETS[] = {
  9, 11, 0, 2, 4, 5, 7
};

void note_num_to_name(int num, char *buf) {
  int oct = (num / 12) - 1;
  const char *note = NOTE_NAMES[num % 12];
  sprintf(buf, "%s%d", note, oct);
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

void format_program(program prog, char *buf) {
  int has_msb = prog.bank_msb != UNDEFINED;
  int has_lsb = prog.bank_lsb != UNDEFINED;
  int has_bank = has_msb || has_lsb;

  sprintf(buf, " %c", has_bank ? '[' : ' ');
  buf += 2;

  if (has_msb)
    sprintf(buf, "%3d", prog.bank_msb);
  else
    strcat(buf, "   ");
  buf += 3;

  sprintf(buf, "%c ", has_bank ? ',' : ' ');
  buf += 2;

  if (has_lsb)
    sprintf(buf, "%3d", prog.bank_lsb);
  else
    strcat(buf, "   ");
  buf += 3;

  sprintf(buf, "%c ", has_bank ? ']' : ' ');
  buf += 2;

  if (prog.prog != UNDEFINED)
    sprintf(buf, " %3d", prog.prog);
  else
    strcat(buf, "    ");
}

void format_controllers(Connection *conn, char *buf) {
  int first = true;

  strcat(buf, " ");
  buf += 1;
  for (int i = 0; i < 128; ++i) {
    Controller *cc = conn->cc_maps[i];
    if (cc == nullptr)
      continue;

    if (first) first = false; else { strcat(buf, ", "); buf += 2; }
    sprintf(buf, "%d", cc->cc_num);
    buf += strlen(buf);

    if (cc->filtered) {
      sprintf(buf, "x");
      buf += 1;
      continue;
    }

    if (cc->cc_num != cc->translated_cc_num) {
      sprintf(buf, "->%d", cc->translated_cc_num);
      buf += strlen(buf);
    }

    if (cc->min_in() != 0 || cc->max_in() != 127
        || cc->min_out() != 0 || cc->max_out() != 127)
    {
      sprintf(buf, " ");
      buf += 1;
      if (cc->pass_through_0) {
        sprintf(buf, "0");
        buf += 1;
      }
      sprintf(buf, " [%d, %d] -> [%d, %d]",
              cc->min_in(), cc->max_in(),
              cc->min_out(), cc->max_out());
      buf += strlen(buf);
      if (cc->pass_through_127) {
        sprintf(buf, "127");
        buf += 3;
      }
    }
  }
  *buf = 0;
}

// Translate floating-point value (to a precision of 0.001) to a string,
// removing trailing zeroes and decimal point if possible.
void format_float(float val, char *buf) {
  sprintf(buf, "%0.2f", val);
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

  fprintf(stderr, "byte value %d is out of range\n", val);
  return false;
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
