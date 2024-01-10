#include "window_utils.h"

wxStaticText * WindowUtils::header_text(wxWindow *parent, const char * const text) {
  wxStaticText *header = new wxStaticText(parent, wxID_ANY, "");
  // NOTE: the space after the '%s' is required. This does the same thing
  // that the TITLE_STR macro does; see the note there.
  header->SetLabelMarkup(wxString::Format("<b>%s </b>", text));
  return header;
}
