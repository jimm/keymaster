#include "catch.hpp"

#define CATCH_CATEGORY "[formatter]"
#include "../src/formatter.h"

TEST_CASE("num to name", CATCH_CATEGORY) {
  char buf[BUFSIZ];

  note_num_to_name(0, buf);
  REQUIRE(strcmp("C-1", buf) == 0);
  note_num_to_name(1, buf);
  REQUIRE(strcmp("C#-1", buf) == 0);
  note_num_to_name(64, buf);
  REQUIRE(strcmp("E4", buf) == 0);
  note_num_to_name(52, buf);
  REQUIRE(strcmp("E3", buf) == 0);
  note_num_to_name(54, buf);
  REQUIRE(strcmp("F#3", buf) == 0);
  note_num_to_name(51, buf);
  REQUIRE(strcmp("D#3", buf) == 0);
  note_num_to_name(127, buf);
  REQUIRE(strcmp("G9", buf) == 0);
}

TEST_CASE("name to num", CATCH_CATEGORY) {
  REQUIRE(note_name_to_num("c-1") == 0);
  REQUIRE(note_name_to_num("C#-1") == 1);
  REQUIRE(note_name_to_num("e4") == 64);
  REQUIRE(note_name_to_num("e3") == 52);
  REQUIRE(note_name_to_num("fs3") == 54);
  REQUIRE(note_name_to_num("f#3") == 54);
  REQUIRE(note_name_to_num("ef3") == 51);
  REQUIRE(note_name_to_num("eb3") == 51);
  REQUIRE(note_name_to_num("g9") == 127);
  REQUIRE(note_name_to_num("G9") == 127);
}

TEST_CASE("name to num given num", CATCH_CATEGORY) {
  REQUIRE(note_name_to_num("0") == 0);
  REQUIRE(note_name_to_num("42") == 42);
}

TEST_CASE("int_from_chars", CATCH_CATEGORY) {
  REQUIRE(int_from_chars("") == 0);
  REQUIRE(int_from_chars("0") == 0);
  REQUIRE(int_from_chars("-12") == -12);
  REQUIRE(int_from_chars("-9912") == -9912);
  REQUIRE(int_from_chars("36") == 36);
  REQUIRE(int_from_chars("0x7f") == 127);
  REQUIRE(int_from_chars("0xff") == 255);
  REQUIRE(int_from_chars("0x00") == 0);
  REQUIRE(int_from_chars("0x01") == 1);
  REQUIRE(int_from_chars("0x123a") == 0x123a);
}

TEST_CASE("message from bytes", CATCH_CATEGORY) {
  char buf[BUFSIZ];

  REQUIRE(message_from_bytes(nullptr) == Pm_Message(0, 0, 0));
  strcpy(buf, "0 0 0");
  REQUIRE(message_from_bytes(buf) == Pm_Message(0, 0, 0));
  strcpy(buf, "0x81, 64, 0x7f");
  REQUIRE(message_from_bytes(buf) == Pm_Message(0x81, 64, 0x7f));
}

TEST_CASE("format float", CATCH_CATEGORY) {
  char buf[BUFSIZ];

  format_float(0.0, buf);
  REQUIRE(strcmp(buf, "0") == 0);

  format_float(120.0, buf);
  REQUIRE(strcmp(buf, "120") == 0);

  format_float(120.030009, buf);
  REQUIRE(strcmp(buf, "120.03") == 0);

  format_float(120.10005, buf);
  REQUIRE(strcmp(buf, "120.1") == 0);

  format_float(120.00005, buf);
  REQUIRE(strcmp(buf, "120") == 0);
}

// We don't have to test individual byte values too much because that's
// covered by hex_to_byte. Make sure we skip non-hex digits and handle error
// conditions.
TEST_CASE("hex to bytes", CATCH_CATEGORY) {
  unsigned char *buf;

  buf = hex_to_bytes("0a ff");
  REQUIRE(buf[0] == 0x0a);
  REQUIRE(buf[1] == 0xff);
  free(buf);

  buf = hex_to_bytes("0a,ff,c");
  REQUIRE(buf[0] == 0x0a);
  REQUIRE(buf[1] == 0xff);
  REQUIRE(buf[2] == 0x0c);
  free(buf);

  buf = hex_to_bytes("3,ff,c2");
  REQUIRE(buf[0] == 0x03);
  REQUIRE(buf[1] == 0xff);
  REQUIRE(buf[2] == 0xc2);
  free(buf);
}

TEST_CASE("bytes to hex", CATCH_CATEGORY) {
  char *buf;
  unsigned char bytes[5] = {1, 2, 127, 0, 15};

  buf = bytes_to_hex(bytes, 5);
  REQUIRE(strcmp(buf, "01027f000f") == 0);
  free(buf);
}
