#include "trigger_list.h"
#include "../keymaster.h"
#include "../formatter.h"

#define CW 24
#define LIST_WIDTH 400
#define SHORT_LIST_HEIGHT 200

const char * const COLUMN_HEADERS[] = {
  "Key", "Input", "Trigger", "Action / Message"
};
const int COLUMN_WIDTHS[] = {
  (int)(1.5*CW), 3*CW, 5*CW, 7*CW
};

TriggerList::TriggerList(wxWindow *parent, wxWindowID id)
  : FrameListCtrl(parent, id, wxDefaultPosition,
                  wxSize(LIST_WIDTH, SHORT_LIST_HEIGHT),
                  wxLC_REPORT | wxLC_SINGLE_SEL)
{
  set_headers();
}

Trigger *TriggerList::selected() {
  long index = GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (index == wxNOT_FOUND)
    return nullptr;

  return KeyMaster_instance()->triggers[index];
}

void TriggerList::update() {
  KeyMaster *km = KeyMaster_instance();

  ClearAll();
  set_headers();

  int row = 0;
  vector<Trigger *> *sorted_triggers =
    sorted_copy(KeyMaster_instance()->triggers);

  for (auto * trigger : *sorted_triggers) {
    int key = trigger->trigger_key_code;

    InsertItem(row, key == UNDEFINED ? ""
               : wxString::Format("F%d", trigger->trigger_key_code - WXK_F1 + 1));

    Input *input = trigger->input();
    SetItem(row, 1, input ? input->name.c_str() : "");

    wxString str;
    if (input != nullptr)
      str = wxString::Format(
        "0x%02x 0x%02x 0x%02x",
        Pm_MessageStatus(trigger->trigger_message),
        Pm_MessageData1(trigger->trigger_message),
        Pm_MessageData2(trigger->trigger_message));
    SetItem(row, 2, str);

    switch (trigger->action) {
    case TA_NEXT_SONG:
      str = "Next Song";
      break;
    case TA_PREV_SONG:
      str = "Prev Song";
      break;
    case TA_NEXT_PATCH:
      str = "Next Patch";
      break;
    case TA_PREV_PATCH:
      str = "Prev Patch";
      break;
    case TA_PANIC:
      str = "All Notes Off";
      break;
    case TA_SUPER_PANIC:
      str = "Super Panic";
      break;
    case TA_TOGGLE_CLOCK:
      str = "Toggle Clock";
      break;
    case TA_MESSAGE:
      str = trigger->output_message->name;
      break;
    }
    SetItem(row, 3, str);
    ++row;
  }

  delete sorted_triggers;
}

void TriggerList::set_headers() {
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    InsertColumn(i, COLUMN_HEADERS[i]);
    SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }
}

// ================ helpers ================

bool songNameComparator(Trigger *t1, Trigger *t2) {
  if (t1->trigger_key_code != UNDEFINED) {
    if (t2->trigger_key_code == UNDEFINED)
      return true;
    return t1->trigger_key_code < t2->trigger_key_code;
  }

  if (t2->trigger_key_code != UNDEFINED)
    return false;

  if (t1->input() != nullptr) {
    if (t2->input() != nullptr)
      return t1->input()->name < t2->input()->name;
    return true;
  }

  if (t2->input() != nullptr)
    return false;

  return true;
}

vector<Trigger *> * TriggerList::sorted_copy(vector<Trigger *> &triggers) {
  vector<Trigger *> *sortable_copy = new vector<Trigger *>(triggers);
  sort(sortable_copy->begin(), sortable_copy->end(), songNameComparator);
  return sortable_copy;
}
