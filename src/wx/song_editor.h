#ifndef SONG_EDITOR_H
#define SONG_EDITOR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "event.h"

using namespace std;

enum {
  ID_SE_Name = 7000,
  ID_SE_BPM,
  ID_SE_StartClock
};

class Song;

class SongEditor : public wxDialog {
public:
  SongEditor(wxWindow *parent, Song *song);

private:
  Song *song;
  wxTextCtrl *name_text;
  wxTextCtrl *text_bpm;
  wxCheckBox *cb_clock_start;

  wxWindow *make_name_panel(wxWindow *parent);
  wxWindow *make_clock_panel(wxWindow *parent);

  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* SONG_EDITOR_H */
