#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/keymaster.h"

#define CATCH_CATEGORY "[keymaster]"
#include "test_helper.h"

void assert_no_start_sent(KeyMaster *km) {
  for (auto& out : km->outputs) {
    for (int i = 0; i < out->num_io_messages; ++i)
      REQUIRE_FALSE(out->io_messages[i] == Pm_Message(0xb0, 7, 0x7f));
  }
  SUCCEED();
}

void assert_no_stop_sent(KeyMaster *km) {
  for (auto& out : km->outputs) {
    for (int i = 0; i < out->num_io_messages; ++i)
      REQUIRE_FALSE(out->io_messages[i] == Pm_Message(0xb2, 7, 0x7f));
  }
  SUCCEED();
}

void assert_start_sent(KeyMaster *km) {
  for (auto& out : km->outputs) {
    for (int i = 0; i < out->num_io_messages; ++i)
      if (out->io_messages[i] == Pm_Message(0xb0, 7, 0x7f))
        return;
  }
  REQUIRE("start message not sent" == nullptr); // FAIL
}

void assert_stop_sent(KeyMaster *km) {
  Output *out = km->outputs[0];
  REQUIRE(!out->real_port());
  for (int i = 0; i < out->num_io_messages; ++i)
    if (out->io_messages[i] == Pm_Message(0xb2, 7, 0x7f))
      return;
  REQUIRE("stop message not sent" == nullptr); // FAIL
}

void clear_out_io_messages(KeyMaster *km) {
  for (auto& out : km->outputs)
    out->num_io_messages = 0;
}

TEST_CASE("send start and stop messages", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  km->start();                  // Another Song
  assert_no_stop_sent(km);
  assert_no_start_sent(km);

  clear_out_io_messages(km);
  km->next_patch();             // second patch in song: has start and stop
  REQUIRE(km->cursor->patch()->start_message->events.size() > 0);
  assert_no_stop_sent(km);
  assert_start_sent(km);

  clear_out_io_messages(km);
  km->next_song();              // To Each His Own
  assert_stop_sent(km);
  assert_no_start_sent(km);

  clear_out_io_messages(km);
  km->prev_song();              // Another Song
  assert_no_stop_sent(km);
  assert_no_start_sent(km);

  km->stop();
  delete km;
}

TEST_CASE("all songs sorted", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  REQUIRE(km->all_songs()->songs[0]->name == "Another Song");
  REQUIRE(km->all_songs()->songs[1]->name == "Song Without Explicit Patch");
  REQUIRE(km->all_songs()->songs[2]->name == "To Each His Own");
  delete km;
}

TEST_CASE("inserted song sorts properly", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  Song *s = new Song(UNDEFINED_ID, "Bees, Bees!");
  km->all_songs()->songs.push_back(s);
  km->sort_all_songs();

  REQUIRE(km->all_songs()->songs[0]->name == "Another Song");
  REQUIRE(km->all_songs()->songs[1]->name == "Bees, Bees!");
  REQUIRE(km->all_songs()->songs[2]->name == "Song Without Explicit Patch");
  REQUIRE(km->all_songs()->songs[3]->name == "To Each His Own");
  delete km;
}

TEST_CASE("inserted song sorts properly, case-sensitively", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  Song *s = new Song(UNDEFINED_ID, "a jar full of bees");
  km->all_songs()->songs.push_back(s);
  km->sort_all_songs();

  REQUIRE(km->all_songs()->songs[0]->name == "Another Song");
  REQUIRE(km->all_songs()->songs[1]->name == "Song Without Explicit Patch");
  REQUIRE(km->all_songs()->songs[2]->name == "To Each His Own");
  REQUIRE(km->all_songs()->songs[3]->name == "a jar full of bees");
  delete km;
}
