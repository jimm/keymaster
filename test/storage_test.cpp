#include <string.h>
#include "catch.hpp"
#include "test_helper.h"
#include "../src/storage.h"

#define CATCH_CATEGORY "[storage]"
// These are the indices of songs in the all-songs list. Different than
// database ID order because the all-songs list is sorted by song name.
#define ANOTHER_INDEX 0
#define SONG_WITHOUT_INDEX 1
#define TO_EACH_INDEX 2

TEST_CASE("storage load", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();

  SECTION("load instruments") {
    REQUIRE(km->inputs.size() == 2);
    REQUIRE(km->outputs.size() == 2);

    Input *in = km->inputs[0];
    REQUIRE(in->name == "first input");

    Output *out = km->outputs[1];
    REQUIRE(out->name == "second output");
  }

  SECTION("load messages") {
    REQUIRE(km->messages.size() == 4); // includes 2 start/stop messages

    Message *msg = km->messages[0];
    REQUIRE(msg->name == "Tune Request");
    REQUIRE(msg->events[0].message == Pm_Message(0xf6, 0, 0));

    msg = km->messages[1];
    REQUIRE(msg->name == "Multiple Note-Offs");
    REQUIRE(msg->events[0].message == Pm_Message(0x80, 64, 0));
    REQUIRE(msg->events[1].message == Pm_Message(0x81, 64, 0));
    REQUIRE(msg->events[2].message == Pm_Message(0x82, 42, 127));
  }

  SECTION("load triggers") {
    Trigger *t;

    REQUIRE(km->triggers.size() == 7);

    // keys
    t = km->triggers[0];
    REQUIRE(t->trigger_key_code == 340);
    REQUIRE(t->trigger_message == Pm_Message(0, 0, 0));
    REQUIRE(t->output_message == nullptr);
    REQUIRE(t->action == TA_PANIC);

    Input *in = km->inputs[0];
    REQUIRE(in->triggers.size() == 5);

    t = in->triggers[0];
    REQUIRE(t->trigger_key_code == UNDEFINED);
    REQUIRE(t->trigger_message == Pm_Message(0xb0, 50, 127));
    REQUIRE(t->action == TA_NEXT_SONG);
    REQUIRE(t->output_message == 0);

    // make sure trigger added to first input
    REQUIRE(find(in->triggers.begin(), in->triggers.end(), t) != in->triggers.end());

    t = in->triggers[4];
    REQUIRE(t->trigger_message == Pm_Message(0xb0, 54, 127));
    REQUIRE(t->action == TA_MESSAGE);
    REQUIRE(t->output_message != 0);
    REQUIRE(t->output_message->name == "Tune Request");
  }

  SECTION("load songs") {
    vector<Song *> &all = km->all_songs->songs;
    REQUIRE(all.size() == 3);

    Song *s = all[0];
    REQUIRE(s->name == "Another Song");

    s = all[1];
    REQUIRE(s->name == "Song Without Explicit Patch");

    s = all[2];
    REQUIRE(s->name == "To Each His Own");
  }

  SECTION("load notes") {
    Song *s = km->all_songs->songs[SONG_WITHOUT_INDEX];
    REQUIRE(s->notes.size() == 0);

    s = km->all_songs->songs[ANOTHER_INDEX];
    REQUIRE(s->notes == "this song has note text\nthat spans multiple lines");
  }

  SECTION("load patches") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX];
    REQUIRE(s->patches.size() == 2);

    Patch *p = s->patches[0];
    REQUIRE(p->name == "Vanilla Through, Filter Two's Sustain");
  }

  SECTION("load start and stop messages") {
    Song *s = km->all_songs->songs[ANOTHER_INDEX];
    Patch *p = s->patches.back();

    REQUIRE(p->start_message->id() == 3);
    REQUIRE(p->stop_message->id() == 4);
  }

  SECTION("load connections") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX]; // To Each His Own
    Patch *p = s->patches[0];          // Two Inputs Merging
    REQUIRE(p->connections.size() == 2);
    Connection *conn = p->connections[0];
    REQUIRE(conn->input == km->inputs[0]);
    REQUIRE(conn->input_chan == CONNECTION_ALL_CHANNELS);
    REQUIRE(conn->output == km->outputs[0]);
    REQUIRE(conn->output_chan == CONNECTION_ALL_CHANNELS);

    s = km->all_songs->songs[ANOTHER_INDEX];  // Another Song
    p = s->patches.back();        // Split Into Two OUtupts
    REQUIRE(p->connections.size() == 2);
    conn = p->connections[0];
    REQUIRE(conn->input_chan == 2);
    REQUIRE(conn->output_chan == 3);

    MessageFilter &mf = conn->message_filter;
    REQUIRE(mf.note);
    REQUIRE(!mf.sysex);
  }

  SECTION("load bank msb lsb") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX]; // To Each His Own
    Patch *p = s->patches[0];          // Vanilla Through
    Connection *conn = p->connections.back();
    REQUIRE(conn->prog.bank_msb == 3);
    REQUIRE(conn->prog.bank_lsb == 2);
  }

  SECTION("load bank lsb only") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX]; // To Each His Own
    Patch *p = s->patches[1];          // One Up One Oct...
    Connection *conn = p->connections.back();
    REQUIRE(conn->prog.bank_msb == -1);
    REQUIRE(conn->prog.bank_lsb == 5);
  }

  SECTION("load prog chg") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX];
    Patch *p = s->patches[0];
    Connection *conn = p->connections.back();
    REQUIRE(conn->prog.prog == 12);
  }

  SECTION("load xpose") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX];
    Patch *p = s->patches[0];
    Connection *conn = p->connections[0];
    REQUIRE(conn->xpose == 0);

    p = s->patches.back();
    conn = p->connections[0];
    REQUIRE(conn->xpose == 12);
    conn = p->connections.back();
    REQUIRE(conn->xpose == -12);
  }

  SECTION("load zone") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX];
    Patch *p = s->patches[0];
    Connection *conn = p->connections[0];
    REQUIRE(conn->zone.low == 0);
    REQUIRE(conn->zone.high == 127);

    s = km->all_songs->songs[ANOTHER_INDEX];  // Another Song
    p = s->patches[1];
    conn = p->connections[0];
    REQUIRE(conn->zone.low == 0);
    REQUIRE(conn->zone.high == 63);

    conn = p->connections[1];
    REQUIRE(conn->zone.low == 64);
    REQUIRE(conn->zone.high == 127);

  }

  SECTION("load controller mappings") {
    Song *s = km->all_songs->songs[TO_EACH_INDEX];
    Patch *p = s->patches[0];
    Connection *conn = p->connections.back();
    REQUIRE(conn->id() == 2);     // sanity check
    REQUIRE(conn->cc_maps[64] != nullptr);
    REQUIRE(conn->cc_maps[64]->filtered == true);

    p = s->patches[1];
    conn = p->connections.front();
    REQUIRE(conn->id() == 3);     // sanity check
    Controller *cc = conn->cc_maps[7];
    REQUIRE(cc != nullptr);
    REQUIRE(cc->translated_cc_num == 10);
    REQUIRE(cc->filtered == false);
    REQUIRE(cc->pass_through_0 == false);
    REQUIRE(cc->pass_through_127 == false);
    REQUIRE(cc->min_in() == 1);
    REQUIRE(cc->max_in() == 120);
    REQUIRE(cc->min_out() == 40);
    REQUIRE(cc->max_out() == 50);
  }

  SECTION("load song list") {
    vector<Song *> &all = km->all_songs->songs;

    REQUIRE(km->set_lists.size() == 3);

    SetList *sl = km->set_lists[1]; // first user-defined song list
    REQUIRE(sl->name == "Set List One");
    REQUIRE(sl->songs.size() == 2);
    REQUIRE(sl->songs[0] == all[TO_EACH_INDEX]);
    REQUIRE(sl->songs.back() == all[ANOTHER_INDEX]);

    sl = km->set_lists[2];       // second user-defined song list
    REQUIRE(sl->name == "Set List Two");
    REQUIRE(sl->songs.size() == 2);
    REQUIRE(sl->songs[0] == all[ANOTHER_INDEX]);
    REQUIRE(sl->songs.back() == all[TO_EACH_INDEX]);
  }

  SECTION("load auto patch") {
    Song *s = km->all_songs->songs[SONG_WITHOUT_INDEX];
    REQUIRE(s->patches.size() == 1);
    Patch *p = s->patches[0];
    REQUIRE(p->name == s->name);
    REQUIRE(p->connections.size() == 1);
    REQUIRE(p->connections[0]->input->name == "first input");
    REQUIRE(p->connections[0]->output->name == "first output");
  }

  delete km;
}

TEST_CASE("initialize", CATCH_CATEGORY) {
  Storage storage(TEST_DB_PATH);
  storage.initialize();
  REQUIRE(storage.has_error() == false);
  
  KeyMaster *km = storage.load(true);
  REQUIRE(km->inputs.size() == 0);
  REQUIRE(km->outputs.size() == 0);
  REQUIRE(km->messages.size() == 0);
  REQUIRE(km->all_songs->songs.size() == 0);
  delete km;
}

TEST_CASE("save", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();

  Storage saver(TEST_DB_PATH "_save_test");
  saver.save(km, true);
  REQUIRE(saver.has_error() == false);

  Storage storage(TEST_DB_PATH "_save_test");
  km = storage.load(true);
  REQUIRE(storage.has_error() == false);

  REQUIRE(km->inputs.size() == 2);
  REQUIRE(km->outputs.size() == 2);
  REQUIRE(km->inputs[0]->name == "first input");
  REQUIRE(km->messages.size() == 4);

  REQUIRE(km->triggers.size() == 7);
  REQUIRE(km->inputs[0]->triggers.size() == 5);

  REQUIRE(km->all_songs->songs.size() == 3);

  Song *song = km->all_songs->songs[0];
  REQUIRE(song->name == "Another Song");
  REQUIRE(song->patches.size() == 2);
  REQUIRE(song->patches[0]->name == "Two Inputs Merging");
  REQUIRE(song->patches[0]->connections.size() == 2);

  REQUIRE(km->set_lists.size() == 3);
  REQUIRE(km->set_lists[0] == km->all_songs);
  REQUIRE(km->set_lists[1]->name == "Set List One");
  REQUIRE(km->set_lists[2]->name == "Set List Two");
  delete km;
}

TEST_CASE("save sets UNDEFINE_ID ids", CATCH_CATEGORY) {
  KeyMaster *km = new KeyMaster();
  km->testing = true;
  km->initialize();

  Song *s = new Song(UNDEFINED_ID, "unnamed");
  km->all_songs->songs.push_back(s);
  REQUIRE(s->id() == UNDEFINED_ID);

  Storage storage(TEST_DB_PATH);
  storage.save(km, true);
  REQUIRE(storage.has_error() == false);
  REQUIRE(s->id() != UNDEFINED_ID);
  REQUIRE(s->id() >= 1LL);
  delete km;
}
