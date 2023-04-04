#include <catch2/catch_all.hpp>
#include "test_helper.h"
#include "../src/clock.h"

#define CATCH_CATEGORY "[clock]"

TEST_CASE("clock bpm math", CATCH_CATEGORY) {
  auto inputs = vector<Input *>();
  auto clock = Clock(inputs);

  REQUIRE(clock.bpm() == 120.0);
  REQUIRE(clock.nanosecs_per_tick == (long)(5.0e8 / 24.0));

  clock.set_bpm(60);
  REQUIRE(clock.bpm() == 60.0);
  REQUIRE(clock.nanosecs_per_tick == (long)(1.0e9 / 24.0));

  clock.set_bpm(240);
  REQUIRE(clock.bpm() == 240.0);
  REQUIRE(clock.nanosecs_per_tick == (long)(2.5e8 / 24.0));
}
