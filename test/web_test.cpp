#include "catch.hpp"
#include "test_helper.h"
#include "../src/web.h"

#define CATCH_CATEGORY "[web]"

TEST_CASE("uri parsing", CATCH_CATEGORY) {
  Web web(nullptr, 0);

  web.parse_params("");
  REQUIRE(web.params.empty() == true);

  web.parse_params("a=3&b=sdfwer");
  REQUIRE(web.params.size() == 2);
  REQUIRE(web.params["a"] == "3");
  REQUIRE(web.params["b"] == "sdfwer");

  web.parse_params("c=hello%20world&de%65=this.is+fine&xyz");
  REQUIRE(web.params.size() == 3);
  REQUIRE(web.params["c"] == "hello world");
  REQUIRE(web.params["dee"] == "this.is fine");
  REQUIRE(web.params["xyz"] == "");
}
