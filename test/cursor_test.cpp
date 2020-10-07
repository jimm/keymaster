#include "catch.hpp"
#include "test_helper.h"
#include "../src/storage.h"

#define CATCH_CATEGORY "[cursor]"

TEST_CASE("init empty", CATCH_CATEGORY) {
  KeyMaster *km = new KeyMaster();
  Cursor *c = km->cursor;
  c->init();
  REQUIRE(c->set_list_index == 0);
  REQUIRE(c->song_index == -1);
  REQUIRE(c->patch_index == -1);
  delete km;
}

TEST_CASE("cursor", CATCH_CATEGORY) {
  KeyMaster *km = load_test_data();
  km->testing = true;
  Cursor *c = km->cursor;
  km->cursor->init();

// ================ initialization

  SECTION("init") {
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 0);
  }

// ================ movement

  SECTION("next patch") {
    c->next_patch();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 1);
  }

  SECTION("next patch at end of song") {
    c->patch_index = 1;
    c->next_patch();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 1);
    REQUIRE(c->patch_index == 0);
  }

  SECTION("next patch at end of set list") {
    c->set_list_index = 1;        // Set List One
    c->song_index = 1;            // Another Song
    c->patch_index = 1;           // Split Into Two Outputs
    c->next_patch();
    REQUIRE(c->set_list_index == 1);
    REQUIRE(c->song_index == 1);
    REQUIRE(c->patch_index == 1);
  }

  SECTION("prev patch") {
    c->patch_index = 1;
    c->prev_patch();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 0);
  }

  SECTION("prev patch start of song") {
    c->song_index = 1;
    c->prev_patch();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 0);
  }

  SECTION("prev patch start of set list") {
    c->prev_patch();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 0);
  }

  SECTION("next song") {
    c->next_song();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 1);
    REQUIRE(c->patch_index == 0);
  }

  SECTION("prev song") {
    c->song_index = 1;
    c->patch_index = 1;
    c->prev_song();
    REQUIRE(c->set_list_index == 0);
    REQUIRE(c->song_index == 0);
    REQUIRE(c->patch_index == 0);
  }

// ================ has_{next,prev}_{song,patch} predicates

  SECTION("has next song true") {
    REQUIRE(c->has_next_song());
  }

  SECTION("has next song false") {
    c->song_index = c->set_list()->songs.size() - 1;
    REQUIRE(!c->has_next_song());
  }

  SECTION("has prev song true") {
    c->song_index = 1;
    REQUIRE(c->has_prev_song());
  }

  SECTION("has prev song false") {
    REQUIRE(!c->has_prev_song());
  }

  SECTION("has next patch true") {
    REQUIRE(c->has_next_patch());
  }

  SECTION("has next patch false") {
    c->song_index = c->set_list()->songs.size() - 1;
    c->patch_index = c->song()->patches.size() - 1;
    REQUIRE(!c->has_next_patch());
  }

  SECTION("has prev patch true") {
    c->patch_index = 1;
    REQUIRE(c->has_prev_patch());
  }

  SECTION("has prev patch false") {
    REQUIRE(!c->has_prev_patch());
  }

  SECTION("has next patch in song true") {
    REQUIRE(c->has_next_patch_in_song());
  }

  SECTION("has next patch in song false") {
    c->song_index = c->set_list()->songs.size() - 1;
    c->patch_index = c->song()->patches.size() - 1;
    REQUIRE(!c->has_next_patch_in_song());
  }

  SECTION("has prev patch in song true") {
    c->song_index = 1;
    c->patch_index = 1;
    REQUIRE(c->has_prev_patch_in_song());
  }

  SECTION("has prev patch in song false") {
    c->song_index = 1;
    REQUIRE(!c->has_prev_patch_in_song());
  }

// ================ defaults

  SECTION("default set list is all songs") {
    REQUIRE(c->set_list() == km->all_songs);
  }

  SECTION("song") {
    REQUIRE(c->song() == km->all_songs->songs[0]);
  }

  SECTION("patch") {
    Song *s = c->song();
    REQUIRE(c->patch() == s->patches[0]);
  }

// ================ goto

  SECTION("goto with regex") {
    SECTION("song") {
      c->goto_song("nother");
      Song *s = c->song();
      REQUIRE(s != nullptr);
      REQUIRE(s->name == "Another Song");

    }

    SECTION("song, no such song") {
      Song *before = c->song();
      REQUIRE(before != nullptr);

      c->goto_song("nosuch");
      Song *s = c->song();
      REQUIRE(s == before);

    }

    SECTION("set list") {
      c->goto_set_list("two");
      SetList *sl = c->set_list();
      REQUIRE(sl != nullptr);
      REQUIRE(sl->name == "Set List Two");

    }

    SECTION("set list, no such set list") {
      SetList *before = c->set_list();
      REQUIRE(before != nullptr);

      c->goto_set_list("nosuch");
      SetList *sl = c->set_list();
      REQUIRE(sl == before);
    }
  }

  SECTION("goto with pointer") {
    SECTION("song") {
      c->goto_set_list("two");

      SECTION("in set list") {
      }

      SECTION("not in set list") {
      }
    }

    SECTION("patch") {
      Song *song = c->song();

      SECTION("in song") {
        Patch *patch = song->patches.back();
        REQUIRE(c->patch() != patch);
        c->goto_patch(patch);
        REQUIRE(c->song() == song);
        REQUIRE(c->patch() == patch);
      }

      SECTION("not in song") {
        Patch *before_patch = c->patch();

        c->goto_patch(nullptr);
        REQUIRE(c->song() == song);
        REQUIRE(c->patch() == before_patch);
      }
    }
  }

  delete km;
}
