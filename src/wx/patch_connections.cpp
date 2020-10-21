#include "patch_connections.h"
#include "events.h"
#include "../keymaster.h"
#include "../cursor.h"
#include "../formatter.h"

#define CW 36

const char * const COLUMN_HEADERS[] = {
  "Input", "Chan", "Output", "Chan", "Zone", "Xpose", "VelCrv", "Prog", "CC Filt/Map"
};
const int COLUMN_WIDTHS[] = {
  3*CW, 1*CW, 3*CW, 1*CW, 2*CW, int(1.5*CW), int(1.5*CW), 3*CW, 6*CW
};
const int COLUMN_ALIGNMENTS[] = {
  // input, chan, output, chan
  wxLIST_FORMAT_LEFT, wxLIST_FORMAT_RIGHT, wxLIST_FORMAT_LEFT, wxLIST_FORMAT_RIGHT,
  // zone, xpose
  wxLIST_FORMAT_LEFT, wxLIST_FORMAT_RIGHT,
  // vel curve, prog, cc filt/map
  wxLIST_FORMAT_RIGHT, wxLIST_FORMAT_RIGHT, wxLIST_FORMAT_LEFT,
};

PatchConnections::PatchConnections(wxWindow *parent, wxWindowID id)
  : FrameListCtrl(parent, id, wxDefaultPosition, wxSize(600, 150),
                  wxLC_REPORT | wxLC_SINGLE_SEL)
{
  set_headers();
}

Connection *PatchConnections::selected() {
  Patch *patch = KeyMaster_instance()->cursor()->patch();
  if (patch == nullptr || patch->connections().empty())
    return nullptr;

  long index = GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  return index == wxNOT_FOUND ? nullptr : patch->connections()[index];
}

void PatchConnections::update() {
  KeyMaster *km = KeyMaster_instance();
  Cursor *cursor = km->cursor();
  Patch *patch = cursor->patch();

  ClearAll();
  set_headers();
  if (patch == nullptr)
    return;

  int i = 0;
  for (auto* conn : patch->connections()) {
    char buf[BUFSIZ];

    InsertItem(i, conn->input()->name().c_str());
    int col = 1;
    SetItem(i, col++, conn->input_chan() == -1 ? "all" : wxString::Format("%d", conn->input_chan() + 1));
    SetItem(i, col++, conn->output()->name().c_str());
    SetItem(i, col++, conn->output_chan() == -1 ? "all" : wxString::Format("%d", conn->output_chan() + 1));

    char buf2[8];
    note_num_to_name(conn->zone_low(), buf);
    note_num_to_name(conn->zone_high(), buf2);
    if (conn->zone_low() != -1 || conn->zone_high() != -1)
      SetItem(i, col++, wxString::Format("%s - %s", buf, buf2));

    if (conn->xpose() != -1)
      SetItem(i, col++, wxString::Format("%c%2d", conn->xpose() < 0 ? '-' : ' ', abs(conn->xpose())));

    switch (conn->velocity_curve()->shape) {
    case Linear:
      SetItem(i, col++, "");
      break;
    case Exponential:
      SetItem(i, col++, "exp");
      break;
    case HalfExponential:
      SetItem(i, col++, "exp/2");
      break;
    case InverseExponential:
      SetItem(i, col++, "-exp");
      break;
    case HalfInverseExponential:
      SetItem(i, col++, "-exp/2");
      break;
    }

    format_program(conn->program_bank_msb(), conn->program_bank_lsb(),
                   conn->program_prog(), buf);
    SetItem(i, col++, buf);

    format_controllers(conn, buf);
    SetItem(i, col++, buf);
    ++i;
  }
}

void PatchConnections::set_headers() {
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    InsertColumn(i, COLUMN_HEADERS[i], COLUMN_ALIGNMENTS[i]);
    SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }
}
