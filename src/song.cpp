#include <stdlib.h>
#include <string.h>
#include "song.h"
#include "vector_utils.h"

Song::Song(sqlite3_int64 id, const char *name)
  : DBObj(id), Named(name), _bpm(120.0), _clock_on_at_start(false)
{
}

Song::~Song() {
  for (auto& patch : _patches)
    delete patch;
}

void Song::set_notes(string &notes) {
  if (_notes != notes) {
    _notes = notes;
    changed();
  }
}

void Song::set_notes(const char *notes) {
  if (_notes != notes) {
    _notes = notes;
    changed();
  }
}

void Song::set_bpm(float bpm) {
  if (_bpm != bpm) {
    _bpm = bpm;
    changed();
  }
}

void Song::set_clock_on_at_start(bool val) {
  if (_clock_on_at_start != val) {
    _clock_on_at_start = val;
    changed();
  }
}

void Song::add_patch(Patch *patch) {
  patch->add_observer(this);
  _patches.push_back(patch);
  changed();
}

void Song::remove_patch(Patch *patch) {
  patch->remove_observer(this);
  erase(_patches, patch);
  delete patch;
  changed();
}

void Song::update(Observable *o, void *arg) {
  changed();
}
