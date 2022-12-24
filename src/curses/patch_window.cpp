#include <stdlib.h>
#include <string.h>
#include "patch_window.h"
#include "../input.h"
#include "../output.h"
#include "../formatter.h"

PatchWindow::PatchWindow(struct rect r, const char *title_prefix, int imaxlen, int omaxlen)
  : Window(r, title_prefix), patch(nullptr), max_input_name_len(imaxlen), max_output_name_len(omaxlen)
{
}

PatchWindow::~PatchWindow() {
}

void PatchWindow::set_contents(Patch *p) {
  title = p ? p->name() : "";
  patch = p;
}

void PatchWindow::draw() {
  Window::draw();
  wmove(win, 1, 1);
  draw_headers();
  if (patch == nullptr)
    return;

  int row = 2;
  for (auto& conn : patch->connections()) {
    wmove(win, row++, 1);
    draw_connection(conn);
  }
}

void PatchWindow::draw_headers() {
  wattron(win, A_REVERSE);
  string str = " Input";
  for (int i = 0; i < (max_input_name_len - 6); ++i)
    str += ' ';
  str += "  Chan | Output";
  for (int i = 0; i < (max_output_name_len - 7); ++i)
    str += ' ';
  str += "  Chan | Zone      | Xpose | Prog            | CC Filters/Maps";
  make_fit(str, 0);
  waddstr(win, str.c_str());
  for (int i = 0; i < getmaxx(win) - 2 - str.length(); ++i)
    waddch(win, ' ');
  wattroff(win, A_REVERSE);
}

void PatchWindow::draw_connection(Connection *conn) {  
  int vis_height = visible_height();
  char buf[BUFSIZ], fitbuf[BUFSIZ];

  format_chans(conn, buf);
  format_zone(conn, buf + strlen(buf));
  format_xpose(conn, buf + strlen(buf));
  format_prog(conn, buf + strlen(buf));
  format_controllers(conn, buf + strlen(buf));

  make_fit(buf, 1, fitbuf);
  waddstr(win, fitbuf);
}

void PatchWindow::format_chans(Connection *conn, char *buf) {
  char inchan[4], outchan[4];

  if (conn->input_chan() == -1)
    strcpy(inchan, "all");
  else
    snprintf(inchan, 4, "%3d", conn->input_chan() + 1);
  if (conn->output_chan() == -1)
    strcpy(outchan, "all");
  else
    snprintf(outchan, 4, "%3d", conn->output_chan() + 1);

  snprintf(buf, BUFSIZ-3, " %*s  %3s | %*s  %3s |",
           max_input_name_len, conn->input()->name().c_str(), inchan,
           max_output_name_len, conn->output()->name().c_str(), outchan);
}

void PatchWindow::format_zone(Connection *conn, char *buf) {
  if (conn->zone_low() != -1 || conn->zone_high() != -1)
    snprintf(buf + strlen(buf), 13, " %3d - %3d |", conn->zone_low(), conn->zone_high());
  else
    strcat(buf, "           |");
}

void PatchWindow::format_xpose(Connection *conn, char *buf) {
  if (conn->xpose() != -1)
    snprintf(buf + strlen(buf), 9, "   %c%2d |", conn->xpose() < 0 ? '-' : ' ', abs(conn->xpose()));
  else
    strcat(buf, "       |");
}

void PatchWindow::format_prog(Connection *conn, char *buf) {
  format_program(conn->program_bank_msb(), conn->program_bank_lsb(),
                 conn->program_prog(), buf + strlen(buf));
  strcat(buf, " |");
}
