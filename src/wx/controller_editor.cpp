#include <wx/gbsizer.h>
#include "controller_editor.h"
#include "macros.h"
#include "../connection.h"
#include "../controller.h"
#include "../formatter.h"

#define CONTAINER_PANEL_ARGS \
  wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_SUNKEN

wxBEGIN_EVENT_TABLE(ControllerEditor, wxDialog)
  EVT_BUTTON(wxID_OK, ControllerEditor::save)
  EVT_CHECKBOX(ID_CMAP_Filtered, ControllerEditor::filtered_changed)
wxEND_EVENT_TABLE()

ControllerEditor::ControllerEditor(wxWindow *parent, Connection *conn,
                                   Controller *cc)
  : wxDialog(parent, wxID_ANY, "Controller Editor", wxDefaultPosition,
             wxSize(480, 500)),
    connection(conn), controller(cc), orig_cc_num(cc->cc_num)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxSizerFlags panel_sizer = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);
  sizer->Add(make_numbers_panel(this), panel_sizer);
  sizer->Add(make_val_mapping_panel(this), panel_sizer);
  sizer->Add(make_filtered_panel(this), panel_sizer);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_sizer);
  SetSizerAndFit(sizer);
  Show(true);
}

wxWindow *ControllerEditor::make_numbers_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, CONTAINER_PANEL_ARGS);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxSizerFlags center_flags =
    wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

  field_sizer->Add(new wxStaticText(p, wxID_ANY, "CC Number"), center_flags);
  cb_cc_number =
    make_cc_number_dropdown(p, ID_CMAP_CCNumber, controller->cc_num, false);
  field_sizer->Add(cb_cc_number, center_flags);

  field_sizer->Add(new wxStaticText(p, wxID_ANY, "Translated CC Number"),
                   center_flags);
  cb_xlated_number =
    make_cc_number_dropdown(p, ID_CMAP_TranslatedNumber,
                            controller->translated_cc_num, false);
  field_sizer->Add(cb_xlated_number, center_flags);

  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Controller Number"));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxComboBox *ControllerEditor::make_cc_number_dropdown(
  wxWindow *parent, wxWindowID id, int curr_val, bool filter_out_existing)
{
  wxArrayString choices;
  for (int i = 0; i < 128; ++i)
    if (!filter_out_existing
        || i == curr_val
        || connection->cc_maps[i] == nullptr)
      choices.Add(wxString::Format("%d", i));

  wxString curr_choice = wxString::Format("%d", curr_val);
  return new wxComboBox(parent, id, curr_choice,
                        wxDefaultPosition, wxDefaultSize, choices,
                        wxCB_READONLY);
}

wxWindow *ControllerEditor::make_val_mapping_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, CONTAINER_PANEL_ARGS);
  wxBoxSizer *input_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *output_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *pass_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxGridBagSizer * const mapping_sizer = new wxGridBagSizer();
  wxGBSpan cell_span = SPAN(1, 1);

  // input min/max

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Inputs"), POS(0, 0), cell_span);

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Min"), POS(0, 1), cell_span);
  tc_min_in = new wxTextCtrl(p, ID_CMAP_MinIn,
                             wxString::Format("%d", controller->min_in()));
  mapping_sizer->Add(tc_min_in, POS(0, 2), cell_span);

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Max"), POS(0, 3), cell_span);
  tc_max_in = new wxTextCtrl(p, ID_CMAP_MaxIn,
                             wxString::Format("%d", controller->max_in()));
  mapping_sizer->Add(tc_max_in, POS(0, 4), cell_span);

  // output min/max

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Outputs"), POS(1, 0), cell_span);

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Min"), POS(1, 1), cell_span);
  tc_min_out = new wxTextCtrl(p, ID_CMAP_MinOut,
                              wxString::Format("%d", controller->min_out()));
  mapping_sizer->Add(tc_min_out, POS(1, 2), cell_span);

  mapping_sizer->Add(new wxStaticText(p, wxID_ANY, "Max"), POS(1, 3), cell_span);
  tc_max_out = new wxTextCtrl(p, ID_CMAP_MaxOut,
                              wxString::Format("%d", controller->max_out()));
  mapping_sizer->Add(tc_max_out, POS(1, 4), cell_span);

  // pass-through booleans

  cb_pass_through_0 = new wxCheckBox(
    p, ID_CMAP_PassThrough0, "Always Pass Through 0");
  cb_pass_through_0->SetValue(controller->pass_through_0);
  mapping_sizer->Add(cb_pass_through_0, POS(2, 1), SPAN(1, 2));
  cb_pass_through_127 = new wxCheckBox(
    p, ID_CMAP_PassThrough127, "Always Pass Through 127");
  cb_pass_through_127->SetValue(controller->pass_through_127);
  mapping_sizer->Add(cb_pass_through_127, POS(2, 3), SPAN(1, 2));

  value_sizer = new wxBoxSizer(wxVERTICAL);
  value_sizer->Add(new wxStaticText(p, wxID_ANY, "Value Mapping"));
  value_sizer->Add(mapping_sizer);

  p->SetSizerAndFit(value_sizer);
  return p;
}

wxWindow *ControllerEditor::make_filtered_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, CONTAINER_PANEL_ARGS);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *field_sizer = new wxBoxSizer(wxHORIZONTAL);

  cb_filtered = new wxCheckBox(p, ID_CMAP_Filtered,
                               "Filter (Block) Controller");
  cb_filtered->SetValue(controller->filtered);
  field_sizer->Add(cb_filtered);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Filter"));
  outer_sizer->Add(field_sizer);

  p->SetSizerAndFit(outer_sizer);
  filtered_changed();
  return p;
}

void ControllerEditor::filtered_changed(wxCommandEvent& _) {
  filtered_changed();
}

void ControllerEditor::filtered_changed() {
  if (cb_filtered->IsChecked())
    value_sizer->Hide((size_t)1);
  else
    value_sizer->Show((size_t)1);
}

void ControllerEditor::save(wxCommandEvent& _) {
  controller->cc_num = int_from_chars(cb_cc_number->GetValue());
  controller->translated_cc_num = int_from_chars(cb_xlated_number->GetValue());
  controller->filtered = cb_filtered->IsChecked();
  controller->set_range(cb_pass_through_0->IsChecked(),
                        cb_pass_through_127->IsChecked(),
                        int_from_chars(tc_min_in->GetValue()),
                        int_from_chars(tc_max_in->GetValue()),
                        int_from_chars(tc_min_out->GetValue()),
                        int_from_chars(tc_max_out->GetValue()));
  if (controller->cc_num != orig_cc_num) {
    connection->remove_cc_num(orig_cc_num);
    connection->set_controller(controller);
  }

  EndModal(wxID_OK);
}
