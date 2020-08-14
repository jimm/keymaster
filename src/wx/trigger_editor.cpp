#include "trigger_editor.h"
#include "../keymaster.h"
#include "../input.h"
#include "../trigger.h"
#include "../formatter.h"

#define MESSAGE_TEXT_WIDTH 100

wxBEGIN_EVENT_TABLE(TriggerEditor, wxDialog)
  EVT_BUTTON(wxID_OK, TriggerEditor::save)
wxEND_EVENT_TABLE()

TriggerEditor::TriggerEditor(wxWindow *parent, Trigger *t)
  : wxDialog(parent, wxID_ANY, "Trigger Editor", wxDefaultPosition, wxSize(480, 500)),
  km(KeyMaster_instance()), trigger(t)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizerFlags label_flags = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);
  wxSizerFlags field_flags = wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT);

  sizer->Add(new wxStaticText(this, wxID_ANY, "Trigger Key"), label_flags);
  sizer->Add(make_key_dropdown(this), field_flags);

  sizer->Add(new wxStaticText(this, wxID_ANY, "Input"), label_flags);
  sizer->Add(make_input_dropdown(this), field_flags);

  sizer->Add(new wxStaticText(this, wxID_ANY, "MIDI Message"), label_flags);
  wxString message_str;
  if (trigger->input() != nullptr)
    message_str = wxString::Format(
      "0x%02x 0x%02x 0x%02x",
      Pm_MessageStatus(trigger->trigger_message),
      Pm_MessageData1(trigger->trigger_message),
      Pm_MessageData2(trigger->trigger_message));

  tc_trigger_message = new wxTextCtrl(this, ID_TE_MessageText, message_str);
  sizer->Add(tc_trigger_message, field_flags);

  sizer->Add(new wxStaticText(this, wxID_ANY, "Action"), label_flags);
  sizer->Add(make_action_dropdown(this), field_flags);

  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), label_flags);

  SetSizerAndFit(sizer);
  Show(true);
}

wxWindow *TriggerEditor::make_key_dropdown(wxWindow *parent) {
  wxArrayString choices;
  int key = trigger->trigger_key_code;

  choices.Add("(No Trigger Key)");
  for (int i = 1; i <= 24; ++i)
    choices.Add(wxString::Format("F%d", i));

  return lc_key = new wxComboBox(
    parent, ID_TE_InputDropdown,
    key == UNDEFINED
      ? "(No Trigger Key)"
      : wxString::Format("F%d", key - WXK_F1 + 1),
    wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
}

wxWindow *TriggerEditor::make_input_dropdown(wxWindow *parent) {
  wxArrayString choices;
  Input *orig_input = trigger->input();

  choices.Add("(No Trigger Instrument)");
  for (auto &input : km->inputs) {
    if (input->enabled || input == trigger->input())
      choices.Add(input->name);
  }

  return lc_input = new wxComboBox(
    parent, ID_TE_InputDropdown,
    orig_input ? orig_input->name : "(No Trigger Instrument)",
    wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
}

wxWindow *TriggerEditor::make_action_dropdown(wxWindow *parent) {
  wxArrayString choices;
  wxString initial_value;

  choices.Add("Next Song");
  choices.Add("Prev Song");
  choices.Add("Next Patch");
  choices.Add("Prev Patch");
  choices.Add("Panic");
  choices.Add("Super Panic");
  for (auto &message : km->messages)
    choices.Add(message->name);

  switch (trigger->action) {
  case TA_NEXT_SONG:
    initial_value = "Next Song";
    break;
  case TA_PREV_SONG:
    initial_value = "Prev Song";
    break;
  case TA_NEXT_PATCH:
    initial_value = "Next Patch";
    break;
  case TA_PREV_PATCH:
    initial_value = "Prev Patch";
    break;
  case TA_PANIC:
    initial_value = "Panic";
    break;
  case TA_SUPER_PANIC:
    initial_value = "Super Panic";
    break;
  case TA_TOGGLE_CLOCK:
    initial_value = "Toggle Clock";
    break;
  case TA_MESSAGE:
    initial_value = trigger->output_message->name;
    break;
  }

  return lc_action =  new wxComboBox(
    parent, ID_TE_ActionDropdown, initial_value, wxDefaultPosition,
    wxDefaultSize, choices, wxCB_READONLY);
}

void TriggerEditor::save(wxCommandEvent& _) {
  // If we have a key, prefer that over input + message
  int index = lc_key->GetCurrentSelection();
  if (index != wxNOT_FOUND && index > 0)
    trigger->set_trigger_key_code(WXK_F1 + index - 1);

  index = lc_input->GetCurrentSelection();
  if (index == wxNOT_FOUND || index == 0)
    trigger->remove_from_input();
  else {
    Input *input = km->inputs[index - 1];
    PmMessage msg = message_from_bytes(tc_trigger_message->GetValue().c_str());
    trigger->set_trigger_message(input, msg);
  }

  wxString val = lc_action->GetValue();
  if (val == "Next Song")
    trigger->action = TA_NEXT_SONG;
  else if (val == "Prev Song")
    trigger->action = TA_PREV_SONG;
  else if (val == "Next Patch")
    trigger->action = TA_NEXT_PATCH;
  else if (val == "Prev Patch")
    trigger->action = TA_PREV_PATCH;
  else if (val == "Panic")
    trigger->action = TA_PANIC;
  else if (val == "Super Panic")
    trigger->action = TA_SUPER_PANIC;
  else if (val == "Toggle Clock")
    trigger->action = TA_TOGGLE_CLOCK;
  else {
    trigger->action = TA_MESSAGE;
    for (auto &msg : km->messages) {
      if (msg->name == val) {
        trigger->output_message = msg;
        break;
      }
    }
  }

  EndModal(wxID_OK);
}
