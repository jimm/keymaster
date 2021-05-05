#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include "keymaster.h"
#include "message.h"
#include "output.h"
#include "formatter.h"

#define NON_REALTIME_STATUS(b) ((b) >= NOTE_OFF && (b) <= EOX)

Message::Message(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name)
{
}

// Reads `buf` which must consist of bytes encoded as two-digit hex strings
// in groups of four bytes and converts those to PmMessages. Each group of
// four represents the bytes in little-endian order (high byte first).
// Replaces the current messages.
void Message::from_chars(const char *buf) {
  _events.clear();
  if (buf == nullptr) {
    changed();
    return;
  }

  while (*buf) {
    PmEvent event = {0, 0};
    string str(buf, 8);
    event.message = stoi(str, 0, 16);
    buf += 8;
    _events.push_back(event);
  }

  changed();
}

// Reads `buf` which must consist of bytes encoded as two-digit hex strings.
// Whitespace is skipped/ignored. Replaces the current messages.
void Message::from_editable_string(const string &str) {
  istringstream istream(str);
  string word;

  _events.clear();
  istream >> word;
  while (istream) {
    PmEvent event = {0, 0};
    int status = hex_to_byte(word.c_str());
    int switch_status = (status < 0xf0 && status >= 0x80) ? (status & 0xf0) : status;
    int data1, data2, data3;
    bool in_sysex = false;

  RESWITCH:
    switch (switch_status) {
    case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER:
    case PITCH_BEND: case SONG_POINTER:
      istream >> word; data1 = hex_to_byte(word.c_str());
      istream >> word; data2 = hex_to_byte(word.c_str());
      event.message = Pm_Message(status, data1, data2);
      _events.push_back(event);
      break;
    case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case SONG_SELECT:
      istream >> word; data1 = hex_to_byte(word.c_str());
      event.message = Pm_Message(status, data1, 0);
      _events.push_back(event);
      break;
    case TUNE_REQUEST:
      event.message = Pm_Message(status, 0, 0);
      _events.push_back(event);
      break;
    case SYSEX:
      in_sysex = true;
      while (in_sysex) {
        data1 = data2 = data3 = 0;
        if (NON_REALTIME_STATUS(status) && status != SYSEX) {
          in_sysex = false;
          if (data1 == EOX) goto SYSEX_EOX;
          goto RESWITCH;
        }

        istream >> word; data1 = hex_to_byte(word.c_str());
        if (NON_REALTIME_STATUS(data1)) {
          in_sysex = false;
          if (data1 == EOX) goto SYSEX_EOX;
          event.message = Pm_Message(status, 0, 0);
          _events.push_back(event);
          status = data1;
          goto RESWITCH;
        }

        istream >> word; data2 = hex_to_byte(word.c_str());
        if (NON_REALTIME_STATUS(data2)) {
          in_sysex = false;
          if (data1 == EOX) goto SYSEX_EOX;
          event.message = Pm_Message(status, data1, 0);
          _events.push_back(event);
          status = data2;
          goto RESWITCH;
        }

        istream >> word; data3 = hex_to_byte(word.c_str());
        if (NON_REALTIME_STATUS(data3)) {
          in_sysex = false;
          if (data1 == EOX) goto SYSEX_EOX;
          event.message = Pm_Message(status, data1, data2);
          _events.push_back(event);
          status = data3;
          goto RESWITCH;
        }

      SYSEX_EOX:
        event.message = Pm_Message(status, data1, data2) + (data3 << 24);
        _events.push_back(event);

        if (in_sysex) {
          istream >> word; status = hex_to_byte(word.c_str());
        }
      }
      break;
    default:
      fprintf(stderr, "bad MIDI data seen in string; expected status byte got %02x\n", status);
      break;
    }
    istream >> word;
  }
  changed();
}

// Returns a string consisting of non-delimited two-digit hex bytes. Every
// PmMessage is converted to four bytes, no matter what the length of the
// encoded MIDI message. The bytes are little-endian, so for every group of
// four bytes the first byte in the string is the highest byte in the
// PmMessage (which is really an int32).
string Message::to_string() {
  string str;
  char buf[9];

  buf[8] = 0;
  for (auto &event : _events) {
    sprintf(buf, "%08x", event.message);
    str += buf;
  }
  return str;
}

// Returns a string consisting of space- and newline-delimited two-digit hex
// numbers. Each message is separated by a newline and each byte within the
// message is separated by a space.
string Message::to_editable_string() {
  char buf[BUFSIZ];
  string str;
  bool in_sysex = false;

  for (auto &event : _events) {
    PmMessage msg = event.message;
    int status = Pm_MessageStatus(msg);
    int switch_status = (status < 0xf0 && status >= 0x80) ? (status & 0xf0) : status;
    int byte;

    switch (switch_status) {
    case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER:
    case PITCH_BEND: case SONG_POINTER:
      if (in_sysex) {
        in_sysex = false;
        str += "f7\n";
      }
      sprintf(buf, "%02x %02x %02x\n", (unsigned char)status,
              (unsigned char)Pm_MessageData1(msg),
              (unsigned char)Pm_MessageData2(msg));
      str += buf;
      break;
    case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case SONG_SELECT:
      if (in_sysex) {
        in_sysex = false;
        str += "f7\n";
      }
      sprintf(buf, "%02x %02x\n", (unsigned char)status,
              (unsigned char)Pm_MessageData1(msg));
      str += buf;
      break;
    case TUNE_REQUEST:
      if (in_sysex) {
        in_sysex = false;
        str += "f7\n";
      }
      str += "f6\n";
      break;
    case SYSEX:
      if (in_sysex)
        str += " f7\n";         // EOX from previous SYSEX
      in_sysex = true;
    PRINT_SYSEX_FOUR_BYTES:
      if (status == EOX) goto EOX_SEEN;
      sprintf(buf, "%02x", status);
      str += buf;

      byte = Pm_MessageData1(msg);
      if (byte == EOX) goto EOX_SEEN;
      sprintf(buf, " %02x", byte);
      str += buf;

      byte = Pm_MessageData2(msg);
      if (byte == EOX) goto EOX_SEEN;
      sprintf(buf, " %02x", byte);
      str += buf;

      byte = (msg >> 24) & 0xff;
      if (byte == EOX) goto EOX_SEEN;
      sprintf(buf, " %02x", byte);
      str += buf;

      break;

    EOX_SEEN:
      in_sysex = false;
      str += " f7\n";
      break;
    default:
      if (in_sysex) {
        str += ' ';
        goto PRINT_SYSEX_FOUR_BYTES;
      }
      sprintf(buf, "%02x", status);
      str += buf;
      break;
    }
  }
  return str;
}

void Message::send_to_all_outputs() {
  for (auto &out : KeyMaster_instance()->outputs())
    send_to(*out);
}

void Message::send_to(Output &out) {
  out.write(_events.data(), _events.size());
}
