#include "catch.hpp"
#include "test_helper.h"
#include "../src/message.h"

#define CATCH_CATEGORY "[message]"

TEST_CASE("from chars", CATCH_CATEGORY) {
  Message m(UNDEFINED_ID, "");

  m.from_chars("00000000");
  REQUIRE(m.events().size() == 1);
  REQUIRE(m.events()[0].message == 0);

  m.from_chars("003412b3");
  REQUIRE(m.events().size() == 1);
  REQUIRE(m.events()[0].message == Pm_Message(0xb3, 0x12, 0x34));

  m.from_chars("003412b37f020170");
  REQUIRE(m.events().size() == 2);
  REQUIRE(m.events()[0].message == Pm_Message(0xb3, 0x12, 0x34));
  REQUIRE(m.events()[1].message == 0x7f020170);
}

TEST_CASE("from editable chars", CATCH_CATEGORY) {
  Message m(UNDEFINED_ID, "");

  m.from_editable_string(string("b2 23 7f\n"));
  REQUIRE(m.events().size() == 1);
  REQUIRE(m.events()[0].message == 0x007f23b2);

  m.from_editable_string(string("b2 23 7f\nf6\nc0 01\n"));
  REQUIRE(m.events().size() == 3);
  REQUIRE(m.events()[0].message == 0x007f23b2);
  REQUIRE(m.events()[1].message == 0x000000f6);
  REQUIRE(m.events()[2].message == 0x000001c0);

  m.from_editable_string(string("b2 23 7f\nf0 01 02 03 04 f7\n81 32 7f\n"));
  REQUIRE(m.events().size() == 4);
  REQUIRE(m.events()[0].message == 0x007f23b2);
  REQUIRE(m.events()[1].message == 0x030201f0);
  REQUIRE(m.events()[2].message == 0x0000f704);
  REQUIRE(m.events()[3].message == 0x007f3281);
}

TEST_CASE("to string", CATCH_CATEGORY) {
  Message m(UNDEFINED_ID, "");

  PmEvent e1 = {0x000000f6, 0};
  m.events().push_back(e1);
  REQUIRE(m.to_string() == "000000f6");

  PmEvent e2 = {0x12345678, 0};
  m.events().push_back(e2);
  REQUIRE(m.to_string() == "000000f612345678");

  PmEvent e3 = {0x000000fa, 0};
  m.events().push_back(e3);
  REQUIRE(m.to_string() == "000000f612345678000000fa");
}

TEST_CASE("to editable string", CATCH_CATEGORY) {
  Message m(UNDEFINED_ID, "");

  PmEvent e1 = {0x000000f6, 0};
  m.events().push_back(e1);
  REQUIRE(m.to_editable_string() == "f6\n");

  PmEvent e2 = {0x007f30b1, 0};
  m.events().push_back(e2);
  REQUIRE(m.to_editable_string() == "f6\nb1 30 7f\n");

  PmEvent e3 = {0x000001c3, 0};
  m.events().push_back(e3);
  REQUIRE(m.to_editable_string() == "f6\nb1 30 7f\nc3 01\n");

  PmEvent e4 = {0x030201f0, 0};
  PmEvent e5 = {0x0000f704, 0};
  m.events().push_back(e4);
  m.events().push_back(e5);
  REQUIRE(m.to_editable_string() == "f6\nb1 30 7f\nc3 01\nf0 01 02 03 04 f7\n");
}
