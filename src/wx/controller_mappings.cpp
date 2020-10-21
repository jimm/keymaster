#include "controller_mappings.h"
#include "../keymaster.h"
#include "../cursor.h"
#include "../formatter.h"

#define CW 60

const char * const COLUMN_HEADERS[] = {
  "CC In", "CC Out", "Filtered", "Pass 0", "Pass 127", "Min In", "Max In", "Min Out", "Max Out"
};

ControllerMappings::ControllerMappings(wxWindow *parent, wxWindowID id, Connection *conn)
  : wxListCtrl(parent, id, wxDefaultPosition, wxSize(600, 150),
               wxLC_REPORT | wxLC_SINGLE_SEL),
    connection(conn)
{
  set_headers();
}

void ControllerMappings::update() {
  ClearAll();
  set_headers();
  if (connection == nullptr)
    return;

  for (int i = 0, row = 0; i < 128; ++i) {
    Controller *controller = connection->cc_map(i);
    if (controller == nullptr)
      continue;

    InsertItem(row, wxString::Format("%d", controller->cc_num()));
    int col = 1;
    SetItem(row, col++, wxString::Format("%d", controller->translated_cc_num()));
    SetItem(row, col++, controller->filtered() ? "yes" : "no");
    if (!controller->filtered()) {
      SetItem(row, col++, controller->pass_through_0() ? "yes" : "no");
      SetItem(row, col++, controller->pass_through_127() ? "yes" : "no");
      SetItem(row, col++, wxString::Format("%d", controller->min_in()));
      SetItem(row, col++, wxString::Format("%d", controller->max_in()));
      SetItem(row, col++, wxString::Format("%d", controller->min_out()));
      SetItem(row, col++, wxString::Format("%d", controller->max_out()));
    }
    ++row;
  }
}

// Returns selected CC mapping controller number, or UNDEFINED if no CC
// mapping is selected.
int ControllerMappings::selected_cc_num() {
  long selected_index = GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (selected_index == wxNOT_FOUND)
    return UNDEFINED;

  for (int i = 0, row = 0; i < 128; ++i) {
    if (connection->cc_map(i) == nullptr)
      continue;

    if (row == int(selected_index))
      return i;
    ++row;
  }
  return UNDEFINED;
}

void ControllerMappings::set_headers() {
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    InsertColumn(i, COLUMN_HEADERS[i]);
    SetColumnWidth(i, CW);
  }
}
