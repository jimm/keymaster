#include "catch.hpp"
#include "test_helper.h"

#define CATCH_CATEGORY "[controller]"

TEST_CASE("out chan", CATCH_CATEGORY) {
  Controller cc(UNDEFINED_ID, 7);
  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 127), 3) == Pm_Message(CONTROLLER + 3, 7, 127));
}

TEST_CASE("filter", CATCH_CATEGORY) {
  Controller cc(UNDEFINED_ID, 7);
  cc.filtered = true;
  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 127), 0) == CONTROLLER_BLOCK);
}

TEST_CASE("map", CATCH_CATEGORY) {
  Controller cc(UNDEFINED_ID, 7);
  cc.translated_cc_num = 10;
  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 127), 0) == Pm_Message(CONTROLLER, 10, 127));
}

TEST_CASE("limit", CATCH_CATEGORY) {
  Controller cc(UNDEFINED_ID, 7);
  cc.set_range(false, true, 1, 120, 1, 120);

  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 0), 0) == CONTROLLER_BLOCK);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 1), 0)) == 1);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 64), 0)) == 64);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 120), 0)) == 120);
  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 121), 0) == CONTROLLER_BLOCK);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 127), 0)) == 127);
}

TEST_CASE("map range", CATCH_CATEGORY) {
  Controller cc(UNDEFINED_ID, 7);
  cc.set_range(true, false, 1, 100, 40, 50);

  // 0 pass
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 0), 0)) == 0);
  // 127 no pass
  REQUIRE(cc.process(Pm_Message(CONTROLLER, 7, 127), 0) == CONTROLLER_BLOCK);

  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 1), 0)) == 40);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 100), 0)) == 50);
  REQUIRE(Pm_MessageData2(cc.process(Pm_Message(CONTROLLER, 7, 50), 0)) == 45);
}
