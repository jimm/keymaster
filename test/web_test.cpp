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

TEST_CASE("status JSON", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  km->cursor()->init();
  Web web(km, 0);
  size_t found;
  bool first;

  string status = web.status_json();

  SECTION("current set list") {
    ostringstream ostr;
    ostr << "\"list\":\"" << km->cursor()->set_list()->name() << '"';
    found = status.find(ostr.str());
    REQUIRE(found != string::npos);
  }

  SECTION("current song") {
    ostringstream ostr;
    Song *song = km->cursor()->song();
    ostr << "\"song\":{\"name\":\"" << song->name() << "\",\"notes\":\""
         << song->notes().replace(song->notes().find("\n"), 1, "\\n")
         << "\",\"patches\":[";
    first = true;
    for (auto &patch : song->patches()) {
      if (first)
        first = false;
      else
        ostr << ',';
      ostr << '"' << patch->name() << '"';
    }
    ostr << "]}";
    found = status.find(ostr.str());
    REQUIRE(found != string::npos);
  }

  SECTION("current patch") {
    ostringstream ostr;
    Patch *patch = km->cursor()->patch();
    ostr << "\"patch\":{\"name\":\"" << patch->name()
         << "\",\"connections\":[";
    found = status.find(ostr.str());
    REQUIRE(found != string::npos);
  }

  SECTION("connection") {
    ostringstream ostr;
    Connection *conn = km->cursor()->patch()->connections()[0];
    ostr << "\"zone\":{\"low\":" << conn->zone_low()
         << ",\"high\":" << conn->zone_high() << "}";
    found = status.find(ostr.str());
    REQUIRE(found != string::npos);
  }

  delete km;
}
