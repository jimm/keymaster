#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
 #include <wx/listctrl.h>
#endif

class WindowUtils {
protected:
  wxStaticText * header_text(wxWindow *parent, const char * const text);
};

#endif /* WINDOW_UTILS_H */
