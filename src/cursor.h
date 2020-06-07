#ifndef CURSOR_H
#define CURSOR_H

#include "keymaster.h"
#include "set_list.h"
#include "song.h"
#include "patch.h"

class Cursor {
public:
  KeyMaster *km;
  int set_list_index;
  int song_index;
  int patch_index;

  Cursor(KeyMaster *km);
  ~Cursor();

  void clear();
  void init();

  SetList *set_list();
  Song *song();
  Patch *patch();

  void next_song();
  void prev_song();
  void next_patch();
  void prev_patch();

  bool has_next_song();
  bool has_prev_song();
  bool has_next_patch();
  bool has_prev_patch();

  bool has_next_patch_in_song();
  bool has_prev_patch_in_song();

  void jump_to_set_list_index(int i);
  void jump_to_song_index(int i);
  void jump_to_patch_index(int i);

  void goto_song(string name_regex);
  void goto_set_list(string name_regex);
};

#endif /* CURSOR_H */
