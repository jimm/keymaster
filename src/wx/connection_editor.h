#ifndef CONNECTION_EDITOR_H
#define CONNECTION_EDITOR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "events.h"

using namespace std;

enum {
  ID_CE_InputDropdown = 4000,
  ID_CE_InputChannel,
  ID_CE_OutputDropdown,
  ID_CE_OutputChannel,
  ID_CE_BankMSB,
  ID_CE_BankLSB,
  ID_CE_Program,
  ID_CE_ZoneLow,
  ID_CE_ZoneHigh,
  ID_CE_Transpose,
  ID_CE_VelocityTranspose,

  // Filters
  ID_CE_PassThroughNote,
  ID_CE_PassThroughPolyPressure,
  ID_CE_PassThroughChanPressure,
  ID_CE_PassThroughProgramChange,
  ID_CE_PassThroughPitchBend,
  ID_CE_PassThroughController,
  ID_CE_PassThroughSongPointer,
  ID_CE_PassThroughSongSelect,
  ID_CE_PassThroughTuneRequest,
  ID_CE_PassThroughSysex,
  ID_CE_PassThroughClock,
  ID_CE_PassThroughStartContinueStop,
  ID_CE_PassThroughSystemReset,

  ID_CE_ControllerMappings,
  ID_CE_AddControllerMapping,
  ID_CE_DelControllerMapping
};

class KeyMaster;
class Instrument;
class Connection;
class Controller;
class ControllerMappings;
class wxListCtrl;
class wxListEvent;

class ConnectionEditor : public wxDialog {
public:
  ConnectionEditor(wxWindow *parent, Connection *connection);

  void update(wxCommandEvent& event) { update(); }
  void update();

private:
  KeyMaster *km;
  Connection *connection;
  wxComboBox *cb_input;
  wxComboBox *cb_input_chan;
  wxComboBox *cb_output;
  wxComboBox *cb_output_chan;
  wxTextCtrl *tc_bank_msb;
  wxTextCtrl *tc_bank_lsb;
  wxTextCtrl *tc_prog;
  wxTextCtrl *tc_zone_low;
  wxTextCtrl *tc_zone_high;
  wxTextCtrl *tc_xpose;
  wxTextCtrl *tc_vel_xpose;

  // filters
  wxCheckBox *cb_pass_note;   // both on and off
  wxCheckBox *cb_pass_poly_pressure;
  wxCheckBox *cb_pass_chan_pressure;
  wxCheckBox *cb_pass_program_change;
  wxCheckBox *cb_pass_pitch_bend;
  wxCheckBox *cb_pass_controller;
  wxCheckBox *cb_pass_song_pointer;
  wxCheckBox *cb_pass_song_select;
  wxCheckBox *cb_pass_tune_request;
  wxCheckBox *cb_pass_sysex;
  wxCheckBox *cb_pass_clock;
  wxCheckBox *cb_pass_start_continue_stop;
  wxCheckBox *cb_pass_system_reset;

  ControllerMappings *lc_cc_mappings;
  wxButton *b_add_ccmap;
  wxButton *b_del_ccmap;

  wxWindow *make_input_panel(wxWindow *parent);
  wxWindow *make_output_panel(wxWindow *parent);
  wxWindow *make_program_panel(wxWindow *parent);
  wxWindow *make_zone_panel(wxWindow *parent);
  wxWindow *make_xpose_panel(wxWindow *parent);
  wxWindow *make_filter_panel(wxWindow *parent);
  wxWindow *make_cc_maps_panel(wxWindow *parent);

  wxWindow *make_instrument_panel(
    wxWindow *parent, wxWindowID inst_id, wxWindowID chan_id,
    const char * const title,
    wxComboBox **instrument_combo_ptr, wxComboBox **chan_combo_ptr,
    vector<Instrument *> &instruments, Instrument *curr_instrument,
    int curr_chan);
  wxComboBox *make_channel_dropdown(
    wxWindow *parent, wxWindowID id, int curr_val,
    const char * const first_choice);

  Instrument *input_from_instrument_list(
    wxComboBox *list, vector<Instrument *> &instruments);
  int channel_from_channel_list(wxComboBox *list);
  int int_or_undefined_from_field(wxTextCtrl *field);

  void edit_controller_mapping(wxListEvent& event);
  bool edit_controller_mapping(Controller *controller);

  void output_channel_changed(wxCommandEvent& _);

  void update_buttons(wxListEvent& event) { update_buttons(); }
  void update_buttons();
  void update_filter_check_boxes();

  void add_controller_mapping(wxCommandEvent& event);
  void del_controller_mapping(wxCommandEvent& event);
  void save(wxCommandEvent& _);

  long selected_cc_map_index();

  wxDECLARE_EVENT_TABLE();
};

#endif /* CONNECTION_EDITOR_H */
