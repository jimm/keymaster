// Defines positions and sizes of windows.
#include <ncurses.h>
#include "geometry.h"

#define MAX_PROMPT_WINDOW_WIDTH 40

inline int TOP_HEIGHT() { return (LINES - 1) * 2 / 3; }
inline int BOT_HEIGHT() { return (LINES - 1) - TOP_HEIGHT(); }
inline int TOP_WIDTH() { return COLS / 3; }
inline int SLS_HEIGHT() { return ((LINES - 1) * 2 / 3) / 3; }
inline int SL_HEIGHT() { return ((LINES - 1) * 2 / 3) - SLS_HEIGHT(); }
inline int PLAY_VIEW_BOT_HEIGHT() { return LINES / 2; }
inline int PLAY_VIEW_TOP_HEIGHT() { return LINES - PLAY_VIEW_BOT_HEIGHT(); }
inline int PLAY_VIEW_LEFT_WIDTH() { return COLS / 3; }
inline int PLAY_VIEW_RIGHT_WIDTH() { return COLS - PLAY_VIEW_LEFT_WIDTH(); }

rect geom_set_list_rect() {
  rect r;

  r.row = 0;
  r.col = 0;
  r.height = SL_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_song_rect() {
  rect r;

  r.row = 0;
  r.col = TOP_WIDTH();
  r.height = SL_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_set_lists_rect() {
  rect r;

  r.row = SL_HEIGHT();
  r.col = 0;
  r.height = SLS_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_messages_rect() {
  rect r;

  r.row = SL_HEIGHT();
  r.col = TOP_WIDTH();
  r.height = SLS_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_triggers_rect() {
  rect r;

  r.row = SL_HEIGHT();
  r.col = TOP_WIDTH() * 2;
  r.height = SLS_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_patch_rect() {
  rect r;

  r.row = ((LINES - 1) * 2 / 3);
  r.col = 0;
  r.height = BOT_HEIGHT();
  r.width = COLS;
  return r;
}

rect geom_message_rect() {
  rect r;

  r.row = LINES - 1;
  r.col = 0;
  r.height = 1;
  r.width = COLS;
  return r;
}

rect geom_info_rect() {
  rect r;

  r.row = 0;
  r.col = TOP_WIDTH() * 2;
  r.height = SL_HEIGHT();
  r.width = COLS - (TOP_WIDTH() * 2);
  return r;
}

rect geom_help_rect() {
  rect r;

  r.row = 3;
  r.col = 3;
  r.height = LINES - 6;
  r.width = COLS - 6;
  return r;
}

rect geom_prompt_rect() {
  rect r;

  r.height = 3;
  r.width = COLS / 2;
  if (r.width > MAX_PROMPT_WINDOW_WIDTH)
    r.width = MAX_PROMPT_WINDOW_WIDTH;
  r.row = LINES / 3;
  r.col = (COLS - r.width) / 2;
  return r;
}

rect geom_play_song_rect() {
  rect r;

  r.row = 0;
  r.col = 0;
  r.height = PLAY_VIEW_TOP_HEIGHT();
  r.width = PLAY_VIEW_LEFT_WIDTH();
  return r;
}

rect geom_play_notes_rect() {
  rect r;

  r.row = 0;
  r.col = PLAY_VIEW_LEFT_WIDTH();
  r.height = PLAY_VIEW_TOP_HEIGHT();
  r.width = PLAY_VIEW_RIGHT_WIDTH();
  return r;
}

rect geom_play_patch_rect() {
  rect r;

  r.row = PLAY_VIEW_TOP_HEIGHT();
  r.col = 0;
  r.height = PLAY_VIEW_BOT_HEIGHT();
  r.width = COLS;
  return r;
}

rect geom_midi_monitor_rect() {
  rect r;

  r.row = 3;
  r.col = (COLS - 38) / 2;
  r.height = LINES - 6;
  r.width = 38;
  return r;
}
