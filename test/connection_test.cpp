#include "catch.hpp"
#include "test_helper.h"

#define CATCH_CATEGORY "[connection]"

TEST_CASE("start sends pc", CATCH_CATEGORY) {
  Input *in = new Input(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Output *out = new Output(UNDEFINED_ID, pmNoDevice, "out 1 port", "out1");
  Connection *conn = new Connection(UNDEFINED_ID, in, 0, out, 1);
  vector<PmMessage> empty;

  conn->prog.prog = 123;
  conn->start();
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(PROGRAM_CHANGE + 1, 123, 0));
  delete conn;
}

TEST_CASE("filter other input chan", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
  REQUIRE(conn->output->num_io_messages == 0);
  delete conn;
}

TEST_CASE("allow all chans", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->input_chan = CONNECTION_ALL_CHANNELS;
  conn->midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127)); /* mutated to output chan */
  delete conn;
}

TEST_CASE("allow all chans in and out", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->input_chan = CONNECTION_ALL_CHANNELS;
  conn->output_chan = CONNECTION_ALL_CHANNELS;
  conn->midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON + 3, 64, 127)); /* out chan not changed */
  delete conn;
}

TEST_CASE("all chans filter controller", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->input_chan = CONNECTION_ALL_CHANNELS;
  conn->output_chan = CONNECTION_ALL_CHANNELS;
  conn->set_controller(new Controller(UNDEFINED_ID, 64));
  conn->cc_maps[64]->filtered = true;
  conn->midi_in(Pm_Message(CONTROLLER + 3, 64, 127));
  REQUIRE(conn->output->num_io_messages == 0);
  delete conn;
}

TEST_CASE("all chans process controller", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->input_chan = 3;
  conn->output_chan = 3;
  conn->set_controller(new Controller(UNDEFINED_ID, 64));
  conn->cc_maps[64]->set_range(false, false, 1, 127, 1, 126);
  conn->midi_in(Pm_Message(CONTROLLER + 3, 64, 127));
  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(CONTROLLER + 3, 64, 126)); /* out value clamped */
  delete conn;
}

TEST_CASE("!xpose", CATCH_CATEGORY) {
  Connection *conn = create_conn();

  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));
  conn->xpose = 12;
  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));
  conn->xpose = -12;
  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));

  REQUIRE(conn->output->num_io_messages == 3);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 64,    127));
  REQUIRE(conn->output->io_messages[1] == Pm_Message(NOTE_ON, 64+12, 127));
  REQUIRE(conn->output->io_messages[2] == Pm_Message(NOTE_ON, 64-12, 127));

  delete conn;
}

TEST_CASE("xpose out of range filteres out note", CATCH_CATEGORY) {
  Connection *conn = create_conn();

  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));
  conn->xpose = 128;
  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));

  REQUIRE(conn->output->num_io_messages ==1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 64,    127));

  delete conn;
}

TEST_CASE("!zone", CATCH_CATEGORY) {
  Connection *conn = create_conn();

  conn->zone.low = 0;
  conn->zone.high = 64;
  conn->midi_in(Pm_Message(NOTE_ON, 48, 127));
  conn->midi_in(Pm_Message(NOTE_OFF, 48, 127));
  conn->midi_in(Pm_Message(NOTE_ON, 76, 127));
  conn->midi_in(Pm_Message(NOTE_OFF, 76, 127));

  REQUIRE(conn->output->num_io_messages == 2);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 48, 127));
  REQUIRE(conn->output->io_messages[1] == Pm_Message(NOTE_OFF, 48, 127));

  delete conn;
}

TEST_CASE("zone poly pressure", CATCH_CATEGORY) {
  Connection *conn = create_conn();

  conn->zone.low = 0;
  conn->zone.high = 64;
  conn->midi_in(Pm_Message(POLY_PRESSURE, 48, 127));
  conn->midi_in(Pm_Message(POLY_PRESSURE, 76, 127));

  REQUIRE(conn->output->num_io_messages == 1);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(POLY_PRESSURE, 48, 127));

  delete conn;
}

TEST_CASE("cc processed", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->set_controller(new Controller(UNDEFINED_ID, 7));
  conn->cc_maps[7]->filtered = true;
  conn->midi_in(Pm_Message(CONTROLLER, 7, 127));

  REQUIRE(conn->output->num_io_messages == 0);

  delete conn;
}

TEST_CASE("filter sysex", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  REQUIRE(conn->pass_through_sysex == false); // check default value

  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));
  conn->midi_in(Pm_Message(SYSEX, 1, 2));
  conn->midi_in(Pm_Message(3, 1, EOX));
  conn->midi_in(Pm_Message(NOTE_OFF, 64, 127));
  REQUIRE(conn->output->num_io_messages == 2);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127));
  REQUIRE(conn->output->io_messages[1] == Pm_Message(NOTE_OFF, 64, 127));
}

TEST_CASE("pass through sysex", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  conn->pass_through_sysex = true;

  conn->midi_in(Pm_Message(NOTE_ON, 64, 127));
  conn->midi_in(Pm_Message(SYSEX, 1, 2));
  conn->midi_in(Pm_Message(3, 1, EOX));
  conn->midi_in(Pm_Message(NOTE_OFF, 64, 127));
  REQUIRE(conn->output->num_io_messages == 4);
  REQUIRE(conn->output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127));
  REQUIRE(conn->output->io_messages[1] == Pm_Message(SYSEX, 1, 2));
  REQUIRE(conn->output->io_messages[2] == Pm_Message(3, 1, EOX));
  REQUIRE(conn->output->io_messages[3] == Pm_Message(NOTE_OFF, 64, 127));
}
