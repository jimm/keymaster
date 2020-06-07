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

TEST_CASE("message from bytes", CATCH_CATEGORY) {
  char buf[BUFSIZ];

  strcpy(buf, "0 0 0");
  REQUIRE(message_from_bytes(buf) == Pm_Message(0, 0, 0));
  strcpy(buf, "0x81, 64, 0x7f");
  REQUIRE(message_from_bytes(buf) == Pm_Message(0x81, 64, 0x7f));
}
