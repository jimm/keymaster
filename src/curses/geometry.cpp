// Defines positions and sizes of windows.
#include <ncurses.h>
#include "geometry.h"

#define MAX_PROMPT_WINDOW_WIDTH 60

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
  return rect { 0, 0, SL_HEIGHT(), TOP_WIDTH() };
}

rect geom_song_rect() {
  return rect { 0, TOP_WIDTH(), SL_HEIGHT(), TOP_WIDTH() };
}

rect geom_set_lists_rect() {
  return rect { SL_HEIGHT(), 0, SLS_HEIGHT(), TOP_WIDTH() };
}

rect geom_messages_rect() {
  return rect { SL_HEIGHT(), TOP_WIDTH(), SLS_HEIGHT(), TOP_WIDTH() };
}

rect geom_triggers_rect() {
  return rect { SL_HEIGHT(), TOP_WIDTH() * 2, SLS_HEIGHT(), COLS - (TOP_WIDTH() * 2) };
}

rect geom_patch_rect() {
  return rect { ((LINES - 1) * 2 / 3), 0, BOT_HEIGHT(), COLS };
}

rect geom_message_rect() {
  return rect { LINES - 1, 0, 1, COLS };
}

rect geom_info_rect() {
  return rect { 0, TOP_WIDTH() * 2, SL_HEIGHT(), COLS - (TOP_WIDTH() * 2) };
}

rect geom_help_rect() {
  return rect { 3, 3, LINES - 6, COLS - 6 };
}

rect geom_prompt_rect() {
  int width = COLS / 2;
  if (width > MAX_PROMPT_WINDOW_WIDTH)
    width = MAX_PROMPT_WINDOW_WIDTH;
  return rect { LINES / 3, (COLS - width) / 2, 3, COLS / 2 };
}

rect geom_play_song_rect() {
  return rect { 0, 0, PLAY_VIEW_TOP_HEIGHT(), PLAY_VIEW_LEFT_WIDTH() };
}

rect geom_play_notes_rect() {
  return rect { 0, PLAY_VIEW_LEFT_WIDTH(), PLAY_VIEW_TOP_HEIGHT(), PLAY_VIEW_RIGHT_WIDTH() };
}

rect geom_play_patch_rect() {
  return rect { PLAY_VIEW_TOP_HEIGHT(), 0, PLAY_VIEW_BOT_HEIGHT(), COLS };
}

rect geom_midi_monitor_rect() {
  return rect { 3, (COLS - 38) / 2, LINES - 6, 38 };
}
