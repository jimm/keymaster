#include "connection_editor.h"
#include "controller_mappings.h"
#include "controller_editor.h"
#include "macros.h"
#include "events.h"
#include "../keymaster.h"
#include "../connection.h"
#include "../formatter.h"

wxBEGIN_EVENT_TABLE(ConnectionEditor, wxDialog)
  EVT_BUTTON(ID_CE_AddControllerMapping, ConnectionEditor::add_controller_mapping)
  EVT_BUTTON(ID_CE_DelControllerMapping, ConnectionEditor::del_controller_mapping)
  EVT_BUTTON(wxID_OK, ConnectionEditor::save)
  EVT_COMBOBOX(ID_CE_OutputChannel, ConnectionEditor::output_channel_changed)
  EVT_LIST_ITEM_ACTIVATED(ID_CE_ControllerMappings, ConnectionEditor::edit_controller_mapping)
  EVT_LIST_ITEM_SELECTED(ID_CE_ControllerMappings, ConnectionEditor::update_buttons)
  EVT_LIST_ITEM_DESELECTED(ID_CE_ControllerMappings, ConnectionEditor::update_buttons)
wxEND_EVENT_TABLE()

ConnectionEditor::ConnectionEditor(wxWindow *parent, Connection *c)
  : wxDialog(parent, wxID_ANY, "Connection Editor", wxDefaultPosition, wxSize(480, 500)),
    km(KeyMaster_instance()), connection(c)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);
  sizer->Add(make_input_panel(this), panel_flags);
  sizer->Add(make_output_panel(this), panel_flags);
  sizer->Add(make_program_panel(this), panel_flags);
  sizer->Add(make_zone_panel(this), panel_flags);
  sizer->Add(make_xpose_panel(this), panel_flags);
  sizer->Add(make_filter_panel(this), panel_flags);
  sizer->Add(make_cc_maps_panel(this), panel_flags);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_flags);

  SetSizerAndFit(sizer);
  update();
  Show(true);
}

wxWindow *ConnectionEditor::make_input_panel(wxWindow *parent) {
  return make_instrument_panel(
    parent, ID_CE_InputDropdown, ID_CE_InputChannel,
    TITLE_STR("Input"), &cb_input, &cb_input_chan,
    reinterpret_cast<vector<Instrument *> &>(km->inputs),
    connection->input, connection->input_chan);
}

wxWindow *ConnectionEditor::make_output_panel(wxWindow *parent) {
  return make_instrument_panel(
    parent, ID_CE_OutputDropdown, ID_CE_OutputChannel,
    TITLE_STR("Output"), &cb_output, &cb_output_chan,
    reinterpret_cast<vector<Instrument *> &>(km->outputs),
    connection->output, connection->output_chan);
}

wxWindow *ConnectionEditor::make_instrument_panel(
  wxWindow *parent, wxWindowID inst_id, wxWindowID chan_id,
  const char * const title,
  wxComboBox **instrument_combo_ptr, wxComboBox **chan_combo_ptr,
  vector<Instrument *> &instruments, Instrument *curr_instrument,
  int curr_chan)
{
  wxArrayString choices;
  wxString curr_output;
  for (auto &instrument : instruments) {
    if (instrument->enabled || instrument == curr_output) {
      choices.Add(instrument->name);
      if (instrument == curr_instrument)
        curr_output = instrument->name;
    }
  }

  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);

  *instrument_combo_ptr = new wxComboBox(
    p, inst_id, curr_output, wxDefaultPosition, wxDefaultSize, choices,
    wxCB_READONLY);
  field_sizer->Add(*instrument_combo_ptr);
  *chan_combo_ptr = make_channel_dropdown(p, chan_id, curr_chan, "All Channels");
  field_sizer->Add(*chan_combo_ptr);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, title));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxComboBox *ConnectionEditor::make_channel_dropdown(
  wxWindow *parent, wxWindowID id, int curr_val, const char * const first_choice)
{
  wxArrayString choices;
  choices.Add(first_choice);
  for (int i = 1; i <= 16; ++i)
    choices.Add(wxString::Format("%d", i));
  wxString curr_choice = curr_val == CONNECTION_ALL_CHANNELS
    ? wxString(first_choice)
    : wxString::Format("%d", curr_val + 1);

  return new wxComboBox(parent, id, curr_choice,
                        wxDefaultPosition, wxDefaultSize, choices,
                        wxCB_READONLY);
}

wxWindow *ConnectionEditor::make_program_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags center_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

  wxString val = connection->prog.bank_msb >= 0
    ? wxString::Format("%d", connection->prog.bank_msb) : "";
  field_sizer->Add(new wxStaticText(p, wxID_ANY, "Bank MSB"), center_flags);
  tc_bank_msb = new wxTextCtrl(p, ID_CE_BankMSB, val);
  field_sizer->Add(tc_bank_msb, center_flags);

  val = connection->prog.bank_lsb >= 0
    ? wxString::Format("%d", connection->prog.bank_lsb) : "";
  field_sizer->Add(new wxStaticText(p, wxID_ANY, "Bank LSB"), center_flags);
  tc_bank_lsb = new wxTextCtrl(p, ID_CE_BankLSB, val);
  field_sizer->Add(tc_bank_lsb, center_flags);

  val = connection->prog.prog >= 0
    ? wxString::Format("%d", connection->prog.prog) : "";
  field_sizer->Add(new wxStaticText(p, wxID_ANY, "PChg"), center_flags);
  tc_prog = new wxTextCtrl(p, ID_CE_Program, val);
  field_sizer->Add(tc_prog, center_flags);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Program Change"));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxWindow *ConnectionEditor::make_zone_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags center_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

  char buf[BUFSIZ];
  note_num_to_name(connection->zone.low, buf);
  wxString val(buf);
  field_sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("Low")), center_flags);
  tc_zone_low = new wxTextCtrl(p, ID_CE_ZoneLow, val);
  field_sizer->Add(tc_zone_low, center_flags);

  note_num_to_name(connection->zone.high, buf);
  val = buf;
  field_sizer->Add(new wxStaticText(p, wxID_ANY, TITLE_STR("High")), center_flags);
  tc_zone_high = new wxTextCtrl(p, ID_CE_ZoneHigh, val);
  field_sizer->Add(tc_zone_high, center_flags);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Zone"), center_flags);
  outer_sizer->Add(field_sizer, center_flags);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxWindow *ConnectionEditor::make_xpose_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxString val = wxString::Format("%d", connection->xpose);
  tc_xpose = new wxTextCtrl(p, ID_CE_Transpose, val);
  field_sizer->Add(tc_xpose);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Transpose"));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxWindow *ConnectionEditor::make_filter_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);

  cb_pass_note = new wxCheckBox(p, ID_CE_PassThroughNote, "Note On/Off");
  cb_pass_poly_pressure = new wxCheckBox(p, ID_CE_PassThroughPolyPressure, "Polyphonic Pressure");
  cb_pass_chan_pressure = new wxCheckBox(p, ID_CE_PassThroughChanPressure, "Channel Pressure");
  cb_pass_program_change = new wxCheckBox(p, ID_CE_PassThroughProgramChange, "Program Change");
  cb_pass_pitch_bend = new wxCheckBox(p, ID_CE_PassThroughPitchBend, "Pitch Bend / Bank MSB/LSB");
  cb_pass_controller = new wxCheckBox(p, ID_CE_PassThroughController, "Controller");
  cb_pass_song_pointer = new wxCheckBox(p, ID_CE_PassThroughSongPointer, "Song Pointer");
  cb_pass_song_select = new wxCheckBox(p, ID_CE_PassThroughSongSelect, "Song Select");
  cb_pass_tune_request = new wxCheckBox(p, ID_CE_PassThroughTuneRequest, "Tune Request");
  cb_pass_sysex = new wxCheckBox(p, ID_CE_PassThroughSysex, "System Exclusive");
  cb_pass_clock = new wxCheckBox(p, ID_CE_PassThroughClock, "Clock");
  cb_pass_start_continue_stop = new wxCheckBox(p, ID_CE_PassThroughStartContinueStop, "Start/Continue/Stop");
  cb_pass_system_reset = new wxCheckBox(p, ID_CE_PassThroughSystemReset, "System Reset");

  wxBoxSizer *channel_sizer = new wxBoxSizer(wxVERTICAL);
  channel_sizer->Add(cb_pass_note);
  channel_sizer->Add(cb_pass_poly_pressure);
  channel_sizer->Add(cb_pass_chan_pressure);
  channel_sizer->Add(cb_pass_program_change);
  channel_sizer->Add(cb_pass_pitch_bend);
  channel_sizer->Add(cb_pass_controller);

  wxBoxSizer *system_sizer = new wxBoxSizer(wxVERTICAL);
  system_sizer->Add(cb_pass_song_pointer);
  system_sizer->Add(cb_pass_song_select);
  system_sizer->Add(cb_pass_tune_request);
  system_sizer->Add(cb_pass_sysex);
  system_sizer->Add(cb_pass_clock);
  system_sizer->Add(cb_pass_start_continue_stop);
  system_sizer->Add(cb_pass_system_reset);

  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Message Filters"));

  wxBoxSizer *side_by_side = new wxBoxSizer(wxHORIZONTAL);
  side_by_side->Add(channel_sizer);
  side_by_side->Add(system_sizer);
  outer_sizer->Add(side_by_side);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxWindow *ConnectionEditor::make_cc_maps_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

  lc_cc_mappings = new ControllerMappings(p, ID_CE_ControllerMappings,
                                          connection);

  b_add_ccmap = new wxButton(p, ID_CE_AddControllerMapping, " + ");
  button_sizer->Add(b_add_ccmap, wxSizerFlags().Left());

  b_del_ccmap = new wxButton(p, ID_CE_DelControllerMapping, " - ");
  button_sizer->Add(b_del_ccmap, wxSizerFlags().Left());

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Controller Mappings"));
  outer_sizer->Add(lc_cc_mappings);
  outer_sizer->Add(button_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

Instrument *ConnectionEditor::input_from_instrument_list(
  wxComboBox *list, vector<Instrument *>& instruments)
{
  return instruments[list->GetCurrentSelection()];
}

int ConnectionEditor::channel_from_channel_list(wxComboBox *list) {
  int n = list->GetCurrentSelection();
  return n == 0 ? CONNECTION_ALL_CHANNELS : (n - 1);
}

int ConnectionEditor::int_or_undefined_from_field(wxTextCtrl *field) {
  wxString val = field->GetValue();
  if (val.empty())
    return UNDEFINED;
  return int_from_chars((const char *)val);
}

void ConnectionEditor::edit_controller_mapping(wxListEvent& event) {
  int controller_num = selected_cc_map_index();
  if (controller_num != wxNOT_FOUND)
    edit_controller_mapping(connection->cc_maps[controller_num]);
}

bool ConnectionEditor::edit_controller_mapping(Controller *controller) {
  if (controller != nullptr)
    if (ControllerEditor(this, connection, controller).ShowModal() == wxID_OK) {
      update();
      return true;
    }
  return false;
}

void ConnectionEditor::update_buttons() {
  bool has_free_cc_map_slot = false;
  for (int i = 0; i < 128 && !has_free_cc_map_slot; ++i)
    if (connection->cc_maps[i] == nullptr)
      has_free_cc_map_slot = true;

  b_add_ccmap->Enable(has_free_cc_map_slot);
  b_del_ccmap->Enable(selected_cc_map_index() != wxNOT_FOUND);
}

void ConnectionEditor::add_controller_mapping(wxCommandEvent& event) {
  int cc_num = -1;
  for (int i = 0; i < 128; ++i) {
    if (connection->cc_maps[i] == nullptr) {
      cc_num = i;
      break;
    }
  }
  if (cc_num == -1)
    return;

  Controller *cc = new Controller(UNDEFINED_ID, cc_num);
  if (edit_controller_mapping(cc)) {
    connection->cc_maps[cc_num] = cc;
    update();
  }
  else
    delete cc;

}

void ConnectionEditor::del_controller_mapping(wxCommandEvent& event) {
  int controller_num = selected_cc_map_index();
  if (controller_num == wxNOT_FOUND)
    return;

  connection->remove_cc_num(controller_num);
  update();
}

void ConnectionEditor::output_channel_changed(wxCommandEvent& _) {
  int n = cb_output_chan->GetCurrentSelection();
  if (n == 0) {                 // all channels
    tc_bank_msb->Disable();
    tc_bank_lsb->Disable();
    tc_prog->Disable();
  }
  else {
    tc_bank_msb->Enable();
    tc_bank_lsb->Enable();
    tc_prog->Enable();
  }
}

void ConnectionEditor::update() {
  lc_cc_mappings->update();
  update_filter_check_boxes();
  update_buttons();
}

void ConnectionEditor::update_filter_check_boxes() {
  MessageFilter &mf = connection->message_filter;
  cb_pass_note->SetValue(mf.note);
  cb_pass_poly_pressure->SetValue(mf.poly_pressure);
  cb_pass_chan_pressure->SetValue(mf.chan_pressure);
  cb_pass_program_change->SetValue(mf.program_change);
  cb_pass_pitch_bend->SetValue(mf.pitch_bend);
  cb_pass_controller->SetValue(mf.controller);
  cb_pass_song_pointer->SetValue(mf.song_pointer);
  cb_pass_song_select->SetValue(mf.song_select);
  cb_pass_tune_request->SetValue(mf.tune_request);
  cb_pass_sysex->SetValue(mf.sysex);
  cb_pass_clock->SetValue(mf.clock);
  cb_pass_start_continue_stop->SetValue(mf.start_continue_stop);
  cb_pass_system_reset->SetValue(mf.system_reset);
}

void ConnectionEditor::save(wxCommandEvent& _) {
  connection->begin_changes();

  connection->input = km->inputs[cb_input->GetCurrentSelection()];
  int n = cb_input_chan->GetCurrentSelection();
  connection->input_chan = (n == 0 ? CONNECTION_ALL_CHANNELS : n - 1);

  connection->output = km->outputs[cb_output->GetCurrentSelection()];
  n = cb_output_chan->GetCurrentSelection();
  connection->output_chan = (n == 0 ? CONNECTION_ALL_CHANNELS : n - 1);

  connection->prog.bank_msb = int_or_undefined_from_field(tc_bank_msb);
  connection->prog.bank_lsb = int_or_undefined_from_field(tc_bank_lsb);
  connection->prog.prog = int_or_undefined_from_field(tc_prog);
  connection->zone.low = note_name_to_num(tc_zone_low->GetValue());
  connection->zone.high = note_name_to_num(tc_zone_high->GetValue());
  connection->xpose = int_from_chars(tc_xpose->GetValue());

  MessageFilter &mf = connection->message_filter;
  mf.note = cb_pass_note->IsChecked();
  mf.poly_pressure = cb_pass_poly_pressure->IsChecked();
  mf.chan_pressure = cb_pass_chan_pressure->IsChecked();
  mf.program_change = cb_pass_program_change->IsChecked();
  mf.pitch_bend = cb_pass_pitch_bend->IsChecked();
  mf.controller = cb_pass_controller->IsChecked();
  mf.song_pointer = cb_pass_song_pointer->IsChecked();
  mf.song_select = cb_pass_song_select->IsChecked();
  mf.tune_request = cb_pass_tune_request->IsChecked();
  mf.sysex = cb_pass_sysex->IsChecked();
  mf.clock = cb_pass_clock->IsChecked();
  mf.start_continue_stop = cb_pass_start_continue_stop->IsChecked();
  mf.system_reset = cb_pass_system_reset->IsChecked();

  // Don't need to update cc_maps because that's done on the fly

  connection->end_changes();
  EndModal(wxID_OK);
}

long ConnectionEditor::selected_cc_map_index() {
  return lc_cc_mappings->GetNextItem(wxNOT_FOUND, wxLIST_NEXT_ALL,
                                     wxLIST_STATE_SELECTED);
}
