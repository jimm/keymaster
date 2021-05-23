#include "catch.hpp"
#include "test_helper.h"
#include "../src/json.h"

#define CATCH_CATEGORY "[json]"

TEST_CASE("JSON", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  km->cursor()->init();
  // size_t found;
  // bool first;

  SECTION("plain string") {
    JSON json;
    REQUIRE(json.encode("foo is plain").str() == "\"foo is plain\"");
  }

  SECTION("single quote and other chars are unharmed") {
    JSON json;
    REQUIRE(json.encode("foo 'bar' is fine! & all is well").str() == "\"foo 'bar' is fine! & all is well\"");
  }

  SECTION("newlines") {
    JSON json;
    REQUIRE(json.encode("foo\nbar\n").str() == "\"foo\\nbar\\n\"");
  }

  SECTION("double quotes") {
    JSON json;
    REQUIRE(json.encode("foo \"bar\"").str() == "\"foo \\\"bar\\\"\"");
  }

  // TODO KeyMaster

  SECTION("message") {
    JSON json;
    json.encode(*km->messages()[2]);
    REQUIRE(json.str() == "{\"name\":\"_start\",\"bytes\":\"00007ab0007f07b0007f07b1\"}");
  }

  SECTION("trigger action via key code") {
    JSON json;
    json.encode(*km->triggers()[0]);
    REQUIRE(json.str() == "{\"key_code\":340,\"input_id\":null,\"trigger_message\":null,\"action\":\"panic\",\"message_id\":null}");
  }

  SECTION("trigger action via message") {
    JSON json;
    json.encode(*km->triggers()[2]);
    REQUIRE(json.str() == "{\"key_code\":null,\"input_id\":1,\"trigger_message\":\"007f32b0\",\"action\":\"next song\",\"message_id\":null}");
  }

  SECTION("trigger a message via a message") {
    JSON json;
    json.encode(*km->triggers()[6]);
    REQUIRE(json.str() == "{\"key_code\":null,\"input_id\":1,\"trigger_message\":\"007f36b0\",\"action\":\"Tune Request\",\"message_id\":1}");
  }

//   SECTION("current set list") {
//     require(json.
//     ostringstream ostr;
//     ostr << "\"set_list\":\"" << km->cursor()->set_list()->name() << '"';
//     found = status.find(ostr.str());
//     REQUIRE(found != string::npos);
//   }

//   SECTION("current song") {
//     ostringstream ostr;
//     Song *song = km->cursor()->song();
//     ostr << "\"song\":{\"name\":\"" << song->name() << "\",\"notes\":\""
//          << song->notes().replace(song->notes().find("\n"), 1, "\\n")
//          << "\",\"patches\":[";
//     first = true;
//     for (auto &patch : song->patches()) {
//       if (first)
//         first = false;
//       else
//         ostr << ',';
//       ostr << '"' << patch->name() << '"';
//     }
//     ostr << "]}";
//     fprintf(stderr, "%s\n", status.c_str()); // DEBUG
//     fprintf(stderr, "%s\n", ostr.str().c_str()); // DEBUG
//     found = status.find(ostr.str());
//     REQUIRE(found != string::npos);
//   }

//   SECTION("current patch") {
//     ostringstream ostr;
//     Patch *patch = km->cursor()->patch();
//     ostr << "\"patch\":{\"name\":\"" << patch->name()
//          << "\",\"connections\":[";
//     found = status.find(ostr.str());
//     REQUIRE(found != string::npos);
//   }

//   SECTION("connection") {
//     ostringstream ostr;
//     Connection *conn = km->cursor()->patch()->connections()[0];
//     ostr << "\"zone\":{\"low\":" << conn->zone_low()
//          << ",\"high\":" << conn->zone_high() << "}";
//     found = status.find(ostr.str());
//     REQUIRE(found != string::npos);
//   }

//   delete km;
// }
}
