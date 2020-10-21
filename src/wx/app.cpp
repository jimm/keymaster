#include <wx/cmdline.h>
#include <portmidi.h>
#include "app.h"
#include "frame.h"
#include "../keymaster.h"
#include "../device.h"

static const wxCmdLineEntryDesc g_cmdLineDesc [] = {
  { wxCMD_LINE_SWITCH, "l", "list-devices", "Display MIDI Devices" },
  { wxCMD_LINE_SWITCH, "i", "initialize", "Output initial KeyMaster file" },
  { wxCMD_LINE_PARAM, nullptr, nullptr, "KeyMaster file", wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_NONE }
};

static App *a_instance = nullptr;

App *app_instance() {
  return a_instance;
}

App::App() {
  a_instance = this;
}

App::~App() {
  if (a_instance == this)
    a_instance = nullptr;
}

void App::show_user_message(string msg) {
  frame->show_user_message(msg);
}

void App::show_user_message(string msg, int clear_secs) {
  frame->show_user_message(msg, clear_secs);
}

// This is the wxWidgets equivalent of the "main" function.
bool App::OnInit() {
  if (!wxApp::OnInit())
    return false;

  init_portmidi();
  frame = new Frame("KeyMaster");
  frame->Show(true);
  SetTopWindow(frame);
  if (command_line_path.IsEmpty())
    frame->initialize();
  else
    frame->load(command_line_path);
  return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser) {
  parser.SetDesc(g_cmdLineDesc);
  parser.SetSwitchChars (wxT("-"));
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser) {
  if (parser.Found("l")) {
    list_all_devices();
    return false;
  }
  if (parser.GetParamCount() > 0)
    command_line_path = parser.GetParam(0);

  return true;
}

int App::FilterEvent(wxEvent &event) {
  if (event.GetEventType() != wxEVT_KEY_DOWN || KeyMaster_instance() == 0)
    return -1;

  return frame->handle_global_key_event((wxKeyEvent &)event);
}

int App::OnExit() {
  KeyMaster *km = KeyMaster_instance();
  // TODO if modified, ask about exiting first
  if (km) km->stop();
  close_portmidi();
  return wxApp::OnExit();
}

void App::init_portmidi() {
  PmError err = Pm_Initialize();
  if (err != 0) {
    fprintf(stderr, "error initializing PortMidi: %s\n", Pm_GetErrorText(err));
    exit(1);
  }

  // Pm_Initialize(), when it looks for default devices, can set errno to a
  // non-zero value. Reinitialize it here.
  errno = 0;
}

void App::close_portmidi() {
  Pm_Terminate();
}

// type is 0 for input, 1 for output
void App::list_all_devices() {
  printf("Inputs:\n");
  for (auto &iter : devices()) {
    const PmDeviceInfo *info = iter.second;
    if (info->input == 1) {
      const char *name = info->name;
      const char *q = (name[0] == ' ' || name[strlen(name)-1] == ' ') ? "\"" : "";
      printf("  %2d: %s%s%s%s\n", iter.first, q, name, q, info->opened ? " (open)" : "");
    }
  }

  printf("Outputs:\n");
  for (auto &iter : devices()) {
    const PmDeviceInfo *info = iter.second;
    if (info->output == 1) {
      const char *name = info->name;
      const char *q = (name[0] == ' ' || name[strlen(name)-1] == ' ') ? "\"" : "";
      printf("  %2d: %s%s%s%s\n", iter.first, q, name, q, info->opened ? " (open)" : "");
    }
  }
}
