#include "catch.hpp"
#include "test_helper.h"
#include "../src/patch.h"

#define CATCH_CATEGORY "[patch]"

TEST_CASE("start msgs", CATCH_CATEGORY) {
  Patch p(UNDEFINED_ID, "test patch");
  Connection *conn = create_conn();
  Message message(UNDEFINED_ID, "A Message");
  message.from_chars("00001232");

  p.connections.push_back(conn);
  p.start_message = &message;

  p.start();
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(0x32, 0x12, 0));
}

TEST_CASE("stop msgs", CATCH_CATEGORY) {
  Patch p(UNDEFINED_ID, "test patch");
  Connection *conn = create_conn();
  Message message(UNDEFINED_ID, "A Message");
  message.from_chars("00001232");

  p.connections.push_back(conn);
  p.stop_message = &message;

  p.running = true;
  p.stop();
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(0x32, 0x12, 0));
}
