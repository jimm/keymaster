#ifndef CURVE_EDITOR_H
#define CURVE_EDITOR_H

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
 #include <wx/listctrl.h>
#endif

class KeyMaster;
class Curve;

enum {
  ID_VE_CurveList = 10000,
  ID_VE_AddCurve,
  ID_VE_DeleteCurve
};

class CurveEditor : public wxDialog {
public:
  CurveEditor(wxWindow *parent, KeyMaster *km);
  ~CurveEditor();

private:
  KeyMaster *km;
  std::vector<Curve *>curves;
  Curve *current_curve;
  wxListBox *lc_curve_list;

  wxWindow *make_curves_panel(wxWindow *parent);
  wxWindow *make_curve_editor_panel(wxWindow *parent);

  void update();

  void fill_in_editor(wxListEvent& event);

  void save(wxCommandEvent& _);

  wxDECLARE_EVENT_TABLE();
};

#endif /* CURVE_EDITOR_H */
