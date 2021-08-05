#include "window_utils.h"

wxStaticText * WindowUtils::header_text(wxWindow *parent, const char * const text) {
  wxStaticText *header = new wxStaticText(parent, wxID_ANY, "");
  header->SetLabelMarkup(wxString::Format("<b>%s</b>", text));
  return header;
}
