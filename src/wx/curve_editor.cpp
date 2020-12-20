#include "curve_editor.h"
#include "../keymaster.h"
#include "../curve.h"

wxBEGIN_EVENT_TABLE(CurveEditor, wxDialog)
  EVT_BUTTON(wxID_OK, CurveEditor::save)
  EVT_LIST_ITEM_SELECTED(ID_VE_CurveList, CurveEditor::fill_in_editor)
wxEND_EVENT_TABLE()

CurveEditor::CurveEditor(wxWindow *parent, KeyMaster *km_instance)
  : wxDialog(parent, wxID_ANY, "Velocity Curve Editor", wxDefaultPosition, wxSize(480, 500)),
    km(km_instance), current_curve(nullptr)
{
  for (auto curve : km->velocity_curves())
    curves.push_back(new Curve(*curve));

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxSizerFlags panel_flags = wxSizerFlags().Expand().Border(wxTOP|wxLEFT|wxRIGHT);
  sizer->Add(make_curves_panel(this), panel_flags);
  sizer->Add(make_curve_editor_panel(this), panel_flags);
  sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), panel_flags);

  SetSizerAndFit(sizer);
  update();
  Show(true);
}

CurveEditor::~CurveEditor() {
  for (auto curve : curves)
    delete curve;
}

wxWindow *CurveEditor::make_curves_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  wxBoxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

  lc_curve_list = new wxListBox(p, ID_VE_CurveList);

  outer_sizer->Add(new wxStaticText(p, wxID_ANY, "Velocity Curves"));
  outer_sizer->Add(lc_curve_list);
  outer_sizer->Add(button_sizer);

  p->SetSizerAndFit(outer_sizer);
  return p;
}

wxWindow *CurveEditor::make_curve_editor_panel(wxWindow *parent) {
  wxPanel *p = new wxPanel(parent, wxID_ANY);
  // TODO
  return p;
}

void CurveEditor::update() {
  lc_curve_list->Clear();
  int i = 0;
  for (auto curve : curves) {
    lc_curve_list->Append(curve->name());
    if (current_curve == curve)
      lc_curve_list->Select(i);
    ++i;
  }

  // TODO edit panel
}

void CurveEditor::fill_in_editor(wxListEvent& event) {
  // TODO
}

void CurveEditor::save(wxCommandEvent& _) {
  for (auto curve : km->velocity_curves())
    delete curve;
  km->velocity_curves() = curves;

  EndModal(wxID_OK);
}
