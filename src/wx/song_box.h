#ifndef SONG_BOX_H
#define SONG_BOX_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "frame_list_box.h"

class Song;

class SongBox : public FrameListBox {
public:
  SongBox(wxWindow *parent, wxWindowID id);

  void update();
  void jump();

private:
  Song *song;
};

#endif /* SONG_BOX_H */
