#include "catch.hpp"
#include "../src/device.h"

#define CATCH_CATEGORY "[device]"

TEST_CASE("device name equality", CATCH_CATEGORY) {
  REQUIRE(device_names_equal("", ""));
  REQUIRE(device_names_equal("  ", ""));
  REQUIRE(device_names_equal("", "  "));
  REQUIRE(device_names_equal("a", "a"));
  REQUIRE(device_names_equal("foo", "foo"));
  REQUIRE(device_names_equal(" foo ", "foo"));
  REQUIRE(device_names_equal("foo", " foo "));
}

TEST_CASE("device name inequality", CATCH_CATEGORY) {
  REQUIRE(!device_names_equal("", "a"));
  REQUIRE(!device_names_equal("a", "b"));
  REQUIRE(!device_names_equal("foo", "foox"));
  REQUIRE(!device_names_equal("foox", "foo"));
}
