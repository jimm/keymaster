#include <catch2/catch_all.hpp>
#include "test_helper.h"
#include "../src/editor.h"

#define CATCH_CATEGORY "[editor]"
#define TEST_FILE "test/testfile.org"

TEST_CASE("create and add message", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  Cursor *c = km->cursor();
  c->init();
  Editor e;

  SECTION("add message") {
    Message *m = e.create_message();
    e.add_message(m);
    REQUIRE(km->messages().back() == m);
  }

  SECTION("create and add trigger") {
    Input *input = km->inputs().front();

    Trigger *t = e.create_trigger(input);
    e.add_trigger(t);
    REQUIRE(km->triggers().back() == t);
  }

  SECTION("create song", CATCH_CATEGORY) {
    Song *created_song = e.create_song();
    REQUIRE(created_song->name() == "Unnamed Song");
    REQUIRE(created_song->patches().size() == 1);
    REQUIRE(created_song->patches().front()->name() == "Unnamed Patch");
  }

  SECTION("add song inserts into current song list", CATCH_CATEGORY) {
    Song *created_song = e.create_song();
    e.add_song(created_song);
    // It'll be last because name is "Unnamed Song" and that's last
    // alphabetically
    REQUIRE(created_song == km->all_songs()->songs().back());
  }

  SECTION("add song inserts into empty song list", CATCH_CATEGORY) {
    SetList *set_list = e.create_set_list();
    e.add_set_list(set_list);
    km->cursor()->set_list_index = km->set_lists().size() - 1;
    // precondition checks
    REQUIRE(set_list == km->cursor()->set_list());
    REQUIRE(set_list->songs().size() == 0);

    Song *created_song = e.create_song();
    e.add_song(created_song);
    REQUIRE(set_list->songs().size() == 1);
    REQUIRE(created_song == set_list->songs().front());
  }

  SECTION("create and add patch", CATCH_CATEGORY) {
    int num_patches = c->song()->patches().size();
    Patch *p = e.create_patch();
    e.add_patch(p, c->song());
    REQUIRE(c->song()->patches().size() == num_patches + 1);
    REQUIRE(c->song()->patches().back() == p);
  }

  SECTION("create and add connection", CATCH_CATEGORY) {
    Patch *patch = c->patch();
    int num_conns = patch->connections().size();
    Connection *conn = e.create_connection(km->inputs().front(), km->outputs().front());
    e.add_connection(conn, patch);
    REQUIRE(patch->connections().size() == num_conns + 1);
    REQUIRE(patch->connections().back() == conn);
  }

  SECTION("create set list", CATCH_CATEGORY) {
    SetList *slist = e.create_set_list();
    e.add_set_list(slist);
    REQUIRE(km->set_lists().back() == slist);
  }

  SECTION("destroy message", CATCH_CATEGORY) {
    int num_messages = km->messages().size();
    e.destroy_message(km->messages().front());
    REQUIRE(km->messages().size() == num_messages - 1);
  }

  SECTION("destroy trigger", CATCH_CATEGORY) {
    Input *input = km->inputs().front();

    int num_triggers = input->triggers().size();
    REQUIRE(num_triggers > 0);    // test precondition check
    e.destroy_trigger(input->triggers().front());
    REQUIRE(input->triggers().size() == num_triggers - 1);
  }

  SECTION("destroy song", CATCH_CATEGORY) {
    int num_songs = km->all_songs()->songs().size();
    e.destroy_song(km->all_songs()->songs().back());
    REQUIRE(km->all_songs()->songs().size() == num_songs - 1);
    REQUIRE(km->set_lists()[1]->songs().size() == 1);
    REQUIRE(km->set_lists()[2]->songs().size() == 1);
  }

  SECTION("create and destroy song", CATCH_CATEGORY) {
    e.create_song();
    e.destroy_song(km->all_songs()->songs().back());
  }

  SECTION("destroy first patch in song with mult. patches", CATCH_CATEGORY) {
    Patch *old_patch = c->patch();
    int num_patches = c->song()->patches().size();

    // precondition check
    REQUIRE(c->song() == km->all_songs()->songs()[0]);
    REQUIRE(c->patch() == c->song()->patches()[0]);

    e.destroy_patch(c->song(), c->patch());

    // old_patch has been deallocated, but we can still check that the current
    // cursor patch is not the same value.
    REQUIRE(c->song() == km->all_songs()->songs()[0]);
    REQUIRE(c->song()->patches().size() == num_patches - 1);

    // current patch is the first patch in the song, but not the same as
    // old_patch
    REQUIRE(c->patch() == c->song()->patches()[0]);
    REQUIRE(c->patch() != old_patch);
  }

  SECTION("destroy last patch in song with mult. patches", CATCH_CATEGORY) {
    int num_patches = c->song()->patches().size();
    c->patch_index = num_patches - 1;
    Patch *old_patch = c->patch();

    // precondition check
    REQUIRE(c->song() == km->all_songs()->songs()[0]);
    REQUIRE(c->patch() == c->song()->patches().back());
    REQUIRE(c->song()->patches().size() == num_patches);

    e.destroy_patch(c->song(), c->patch());

    // old_patch has been deallocated, but we can still check that the current
    // cursor patch is not the same value.
    REQUIRE(c->song() == km->all_songs()->songs()[0]);
    REQUIRE(c->song()->patches().size() == num_patches - 1);

    // current patch is the first patch in the song, but not the same as
    // old_patch
    REQUIRE(c->patch() == c->song()->patches().back());
    REQUIRE(c->patch() != old_patch);
  }

  SECTION("destroy connection", CATCH_CATEGORY) {
    Patch *p = km->cursor()->patch();
    Connection *old_conn = p->connections().back();
    int num_conns = p->connections().size();

    e.destroy_connection(p, old_conn);
    REQUIRE(p->connections().size() == num_conns - 1);
    for (auto &conn : p->connections())
      if (conn == old_conn)
        FAIL("old_conn was not removed");
  }

  SECTION("destroy set list", CATCH_CATEGORY) {
    SetList *old_set_list = km->set_lists().back();
    int num_set_lists = km->set_lists().size();

    e.destroy_set_list(old_set_list);

    REQUIRE(km->set_lists().size() == num_set_lists - 1);
    for (auto &slist : km->set_lists())
      if (slist == old_set_list)
        FAIL("old_set_list was not removed");
  }

  SECTION("add then destroy patches", CATCH_CATEGORY) {
    Song *song = c->song();
    REQUIRE(song->patches().size() == 2); // precondition check

    Patch *created1 = e.create_patch();
    Patch *created2 = e.create_patch();
    e.add_patch(created1, song);
    e.add_patch(created2, song);

    REQUIRE(created1 != created2);
    REQUIRE(created2 == song->patches().back());

    REQUIRE(song->patches().size() == 4);
    REQUIRE(c->patch() == song->patches().back()); // cursor moves to last added patch

    c->patch_index = 2;              // first created patch
    REQUIRE(c->patch() == created1); // precondition check

    e.destroy_patch(song, c->patch());
    REQUIRE(c->patch_index == 2);
    REQUIRE(c->patch() == created2);

    e.destroy_patch(song, c->patch());
    REQUIRE(c->song() == song);
    REQUIRE(c->patch_index == 1);
  }

  delete km;
}
