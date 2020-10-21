#include <stdlib.h>
#include <string.h>
#include "set_list.h"
#include "vector_utils.h"

SetList::SetList(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name)
{
}

SetList::~SetList() {
}

void SetList::add_song(Song *song) {
  song->add_observer(this);
  _songs.push_back(song);
  changed();
}

void SetList::remove_song(Song *song) {
  song->remove_observer(this);
  int old_num_songs = _songs.size();
  erase(_songs, song);
  if (_songs.size() != old_num_songs)
    changed();
}

void SetList::set_songs(vector<Song *>&other_songs) {
  _songs = other_songs;
  changed();
}

void SetList::update(Observable *o, void *arg) {
  changed();
}
