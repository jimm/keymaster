#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct rect {
  int row, col, height, width;
} rect;

rect geom_set_lists_rect();
rect geom_set_list_rect();
rect geom_song_rect();
rect geom_trigger_rect();
rect geom_patch_rect();
rect geom_message_rect();
rect geom_info_rect();
rect geom_help_rect();
rect geom_prompt_rect();

rect geom_play_song_rect();
rect geom_play_notes_rect();
rect geom_play_patch_rect();

rect geom_midi_monitor_rect();

#endif /* GEOMETRY_H */
