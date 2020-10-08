#include "patch_connections.h"
#include "events.h"
#include "../keymaster.h"
#include "../cursor.h"
#include "../formatter.h"

#define CW 36

const char * const COLUMN_HEADERS[] = {
  "Input", "Chan", "Output", "Chan", "Zone", "Xpose", "Prog", "CC Filt/Map"
};
const int COLUMN_WIDTHS[] = {
  3*CW, 1*CW, 3*CW, 1*CW, 2*CW, 1*CW, 3*CW, 6*CW
};

PatchConnections::PatchConnections(wxWindow *parent, wxWindowID id)
  : FrameListCtrl(parent, id, wxDefaultPosition, wxSize(600, 150),
                  wxLC_REPORT | wxLC_SINGLE_SEL)
{
  set_headers();
}

Connection *PatchConnections::selected() {
  Patch *patch = KeyMaster_instance()->cursor->patch();
  if (patch == nullptr || patch->connections.empty())
    return nullptr;

  long index = GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  return index == wxNOT_FOUND ? nullptr : patch->connections[index];
}

void PatchConnections::update() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor;
  Patch *patch = cursor->patch();

  ClearAll();
  set_headers();
  if (patch == nullptr)
    return;

  int i = 0;
  for (auto* conn : patch->connections) {
    char buf[BUFSIZ];

    InsertItem(i, conn->input->name.c_str());
    SetItem(i, 1, conn->input_chan == -1 ? "all" : wxString::Format("%d", conn->input_chan + 1));
    SetItem(i, 2, conn->output->name.c_str());
    SetItem(i, 3, conn->output_chan == -1 ? "all" : wxString::Format("%d", conn->output_chan + 1));

    char buf2[8];
    note_num_to_name(conn->zone.low, buf);
    note_num_to_name(conn->zone.high, buf2);
    if (conn->zone.low != -1 || conn->zone.high != -1)
      SetItem(i, 4, wxString::Format("%s - %s", buf, buf2));

    if (conn->xpose != -1)
      SetItem(i, 5, wxString::Format("%c%2d", conn->xpose < 0 ? '-' : ' ', abs(conn->xpose)));

    format_program(conn->prog, buf);
    SetItem(i, 6, buf);

    format_controllers(conn, buf);
    SetItem(i, 7, buf);
    ++i;
  }
}

void PatchConnections::set_headers() {
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    InsertColumn(i, COLUMN_HEADERS[i]);
    SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }
}
