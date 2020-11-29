#include "catch.hpp"
#include "test_helper.h"
#include "../src/clock.h"

#define CATCH_CATEGORY "[clock]"

TEST_CASE("clock bpm math", CATCH_CATEGORY) {
  auto inputs = vector<Input *>();
  auto clock = Clock(inputs);

  REQUIRE(clock.bpm() == 120.0);
  REQUIRE(clock.microsecs_per_tick == (long)(5.0e5 / (float)CLOCK_TICKS_PER_QUARTER_NOTE));

  clock.set_bpm(60);
  REQUIRE(clock.bpm() == 60.0);
  REQUIRE(clock.microsecs_per_tick == (long)(1.0e6 / (float)CLOCK_TICKS_PER_QUARTER_NOTE));

  clock.set_bpm(240);
  REQUIRE(clock.bpm() == 240.0);
  REQUIRE(clock.microsecs_per_tick == (long)(2.5e5 / (float)CLOCK_TICKS_PER_QUARTER_NOTE));
}
