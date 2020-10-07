#include "consts.h"
#include "editor.h"
#include "keymaster.h"
#include "cursor.h"

#define ITER(type) vector<type *>::iterator

Editor::Editor(KeyMaster *key_master)
  : km(key_master ? key_master : KeyMaster_instance())
{
}

Message *Editor::create_message() {
  return new Message(UNDEFINED_ID, "Unnamed Message");
}

Trigger *Editor::create_trigger(Input *input) {
  return new Trigger(UNDEFINED_ID, TA_NEXT_PATCH, nullptr);
}

Song *Editor::create_song() {
  Song *song = new Song(UNDEFINED_ID, "Unnamed Song");
  // TODO consolidate with Storage::create_default_patch
  Patch *patch = create_patch();
  add_patch(patch, song);
  return song;
}

Patch *Editor::create_patch() {
  return new Patch(UNDEFINED_ID, "Unnamed Patch");
}

Connection *Editor::create_connection(Input *input, Output *output)
{
  return new Connection(UNDEFINED_ID, input, CONNECTION_ALL_CHANNELS,
                        output, CONNECTION_ALL_CHANNELS);
}

SetList *Editor::create_set_list() {
  return new SetList(UNDEFINED_ID, "Unnamed Set List");
}

void Editor::add_message(Message *message) {
  km->messages.push_back(message);
}

void Editor::add_trigger(Trigger *trigger) {
  km->triggers.push_back(trigger);
}

void Editor::add_song(Song *song) {
  km->all_songs->songs.push_back(song);
  km->sort_all_songs();

  SetList *curr_set_list = km->cursor->set_list();
  if (curr_set_list == km->all_songs) {
    km->goto_song(song);
    return;
  }

  vector<Song *> &slist = curr_set_list->songs;
  Song *curr_song = km->cursor->song();
  if (curr_song == nullptr)
    slist.push_back(song);
  else {
    for (ITER(Song) iter = slist.begin(); iter != slist.end(); ++iter) {
      if (*iter == curr_song) {
        slist.insert(++iter, song);
        break;
      }
    }
  }

  km->goto_song(song);
}

void Editor::add_patch(Patch *patch) {
  add_patch(patch, km->cursor->song());
}

void Editor::add_patch(Patch *patch, Song *song) {
  song->patches.push_back(patch);
  km->goto_patch(patch);
}

void Editor::add_connection(Connection *connection, Patch *patch)
{
  if (patch != nullptr)
    patch->add_connection(connection);
}

void Editor::add_set_list(SetList *set_list) {
  km->set_lists.push_back(set_list);
}

void Editor::destroy_message(Message *message) {
  for (ITER(Message) i = km->messages.begin(); i != km->messages.end(); ++i) {
    if (*i == message) {
      km->messages.erase(i);
      delete message;
      return;
    }
  }
}

void Editor::destroy_trigger(Trigger *trigger) {
  trigger->remove_from_input();

  for (ITER(Trigger) i = km->triggers.begin(); i != km->triggers.end(); ++i) {
    if (*i == trigger) {
      km->triggers.erase(i);
      delete trigger;
      return;
    }
  }
}

// Returns true if `message` is not used anywhere.
bool Editor::ok_to_destroy_message(Message *message) {
  if (message == nullptr)
    return false;

  for (auto &song : km->all_songs->songs)
    for (auto &patch : song->patches)
      if (patch->start_message == message || patch->stop_message == message)
        return false;

  for (auto &trigger : km->triggers)
    if (trigger->output_message == message)
      return false;

  return true;
}

bool Editor::ok_to_destroy_trigger(Trigger *trigger) {
  return true;
}

bool Editor::ok_to_destroy_song(Song *song) {
  return true;
}

bool Editor::ok_to_destroy_patch(Song *song, Patch *patch) {
  return song != nullptr
    && song->patches.size() >= 1;
}

bool Editor::ok_to_destroy_connection(Patch *patch, Connection *connection) {
  return patch != nullptr
    && connection != nullptr
    && patch->connections.size() >= 1;
}

bool Editor::ok_to_destroy_set_list(SetList *set_list) {
  return set_list != nullptr
    && set_list != km->all_songs;
}

void Editor::destroy_song(Song *song) {
  if (km->cursor->patch())
    km->cursor->patch()->stop();
  move_away_from_song(song);

  for (auto &set_list : km->set_lists)
    remove_song_from_set_list(song, set_list);

  for (ITER(Song) i = km->all_songs->songs.begin();
       i != km->all_songs->songs.end();
       ++i)
  {
    if (*i == song) {
      km->all_songs->songs.erase(i);
      delete song;
      return;                   // only appears once in all_songs list
    }
  }

  if (km->cursor->patch())
    km->cursor->patch()->start();
}

// Will not destroy the only patch in a song.
void Editor::destroy_patch(Song *song, Patch *patch) {
  if (song->patches.size() <= 1)
    return;

  if (km->cursor->patch())
    km->cursor->patch()->stop();
  move_away_from_patch(song, patch);

  for (ITER(Patch) i = song->patches.begin(); i != song->patches.end(); ++i) {
    if (*i == patch) {
      song->patches.erase(i);
      delete patch;
      break;
    }
  }

  if (km->cursor->patch())
    km->cursor->patch()->start();
}

void Editor::destroy_connection(Patch *patch, Connection *connection) {
  patch->remove_connection(connection);
  delete connection;
}

void Editor::destroy_set_list(SetList *set_list) {
  if (set_list == km->cursor->set_list())
    km->cursor->goto_set_list("All Songs");

  for (ITER(SetList) i = km->set_lists.begin(); i != km->set_lists.end(); ++i) {
    if (*i == set_list) {
      km->set_lists.erase(i);
      delete set_list;
      return;
    }
  }
}

void Editor::remove_song_from_set_list(Song *song, SetList *set_list) {
  for (ITER(SetList) i = km->set_lists.begin(); i != km->set_lists.end(); ++i) {
    SetList *slist = *i;
    if (set_list == slist) {
      for (ITER(Song) j = slist->songs.begin(); j != slist->songs.end(); ++j) {
        if (*j == song) {
          slist->songs.erase(j);
          break;
        }
      }
    }
  }
}

// If `song` is not the current song, does nothing. Else tries to move to
// the next song (see comment though) or, if there isn't one, the prev song.
// If both those fail (this is the only song in the current set list) then
// reinits the cursor.
void Editor::move_away_from_song(Song *song) {
  Cursor *cursor = km->cursor;

  if (song != cursor->song())
    return;

  if (cursor->has_next_song()) {
    // Don't move to next song because this one will be deleted and the
    // cursor index will point to the next song. Do set the cursor's patch
    // index back to 0.
    cursor->patch_index = 0;
    return;
  }

  if (cursor->has_prev_song()) {
    km->prev_song();
    return;
  }

  // Nowhere to move. Reini the cursor;
  cursor->init();
}

// If `patch` is not the current patch, does nothing. Else tries to move to
// the next patch or, if there isn't one, the prev patch. If both those fail
// (this is the only patch in the current song) then calls
// move_away_from_song.
void Editor::move_away_from_patch(Song *song, Patch *patch) {
  Cursor *cursor = km->cursor;

  if (patch != cursor->patch())
    return;

  if (cursor->has_next_patch_in_song()) {
    // Do nothing. We'll remove the patch from the song. The cursor patch
    // index will remain the same, so it will point to the patch that is
    // currently after this one.
    return;
  }

  if (cursor->has_prev_patch_in_song()) {
    km->prev_patch();
    return;
  }

  move_away_from_song(song);
}
