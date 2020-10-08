#include "catch.hpp"
#include "test_helper.h"

#define CATCH_CATEGORY "[connection]"

TEST_CASE("start sends pc", CATCH_CATEGORY) {
  Input in(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Output out(UNDEFINED_ID, pmNoDevice, "out 1 port", "out1");
  Connection conn(UNDEFINED_ID, &in, 0, &out, 1);
  vector<PmMessage> empty;

  conn.prog.prog = 123;
  conn.start();
  REQUIRE(conn.output->num_io_messages == 1);
  REQUIRE(conn.output->io_messages[0] == Pm_Message(PROGRAM_CHANGE + 1, 123, 0));
}

TEST_CASE("filter and modify", CATCH_CATEGORY) {
  Input in(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Output out(UNDEFINED_ID, pmNoDevice, "out 1 port", "out1");
  Connection conn(UNDEFINED_ID, &in, 0, &out, 0);
  conn.start();

  SECTION("filter other input chan") {
    conn.output->clear();
    conn.midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
    REQUIRE(conn.output->num_io_messages == 0);
  }

  SECTION("allow all chans") {
    conn.output->clear();
    conn.input_chan = CONNECTION_ALL_CHANNELS;
    conn.midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
    REQUIRE(conn.output->num_io_messages == 1);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127)); /* mutated to output chan */
  }

  SECTION("allow all chans in and out") {
    conn.output->clear();
    conn.input_chan = CONNECTION_ALL_CHANNELS;
    conn.output_chan = CONNECTION_ALL_CHANNELS;
    conn.midi_in(Pm_Message(NOTE_ON + 3, 64, 127));
    REQUIRE(conn.output->num_io_messages == 1);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON + 3, 64, 127)); /* out chan not changed */
  }

  SECTION("all chans filter controller") {
    conn.output->clear();
    conn.input_chan = CONNECTION_ALL_CHANNELS;
    conn.output_chan = CONNECTION_ALL_CHANNELS;
    conn.set_controller(new Controller(UNDEFINED_ID, 64));
    conn.cc_maps[64]->filtered = true;
    conn.midi_in(Pm_Message(CONTROLLER + 3, 64, 127));
    REQUIRE(conn.output->num_io_messages == 0);
  }

  SECTION("all chans process controller") {
    conn.output->clear();
    conn.input_chan = 3;
    conn.output_chan = 3;
    conn.set_controller(new Controller(UNDEFINED_ID, 64));
    conn.cc_maps[64]->set_range(false, false, 1, 127, 1, 126);
    conn.midi_in(Pm_Message(CONTROLLER + 3, 64, 127));
    REQUIRE(conn.output->num_io_messages == 1);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(CONTROLLER + 3, 64, 126)); /* out value clamped */
  }

  SECTION("!xpose") {
    conn.output->clear();

    conn.midi_in(Pm_Message(NOTE_ON, 64, 127));
    conn.xpose = 12;
    conn.midi_in(Pm_Message(NOTE_ON, 64, 127));
    conn.xpose = -12;
    conn.midi_in(Pm_Message(NOTE_ON, 64, 127));

    REQUIRE(conn.output->num_io_messages == 3);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 64,    127));
    REQUIRE(conn.output->io_messages[1] == Pm_Message(NOTE_ON, 64+12, 127));
    REQUIRE(conn.output->io_messages[2] == Pm_Message(NOTE_ON, 64-12, 127));

  }

  SECTION("xpose out of range filteres out note") {
    conn.output->clear();

    conn.midi_in(Pm_Message(NOTE_ON, 64, 127));
    conn.xpose = 128;
    conn.midi_in(Pm_Message(NOTE_ON, 64, 127));

    REQUIRE(conn.output->num_io_messages ==1);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 64,    127));

  }

  SECTION("!zone") {
    conn.output->clear();

    conn.zone.low = 0;
    conn.zone.high = 64;
    conn.midi_in(Pm_Message(NOTE_ON, 48, 127));
    conn.midi_in(Pm_Message(NOTE_OFF, 48, 127));
    conn.midi_in(Pm_Message(NOTE_ON, 76, 127));
    conn.midi_in(Pm_Message(NOTE_OFF, 76, 127));

    REQUIRE(conn.output->num_io_messages == 2);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 48, 127));
    REQUIRE(conn.output->io_messages[1] == Pm_Message(NOTE_OFF, 48, 127));

  }

  SECTION("zone poly pressure") {
    conn.output->clear();

    conn.zone.low = 0;
    conn.zone.high = 64;
    conn.midi_in(Pm_Message(POLY_PRESSURE, 48, 127));
    conn.midi_in(Pm_Message(POLY_PRESSURE, 76, 127));

    REQUIRE(conn.output->num_io_messages == 1);
    REQUIRE(conn.output->io_messages[0] == Pm_Message(POLY_PRESSURE, 48, 127));

  }

  SECTION("cc processed") {
    conn.output->clear();
    conn.set_controller(new Controller(UNDEFINED_ID, 7));
    conn.cc_maps[7]->filtered = true;
    conn.midi_in(Pm_Message(CONTROLLER, 7, 127));

    REQUIRE(conn.output->num_io_messages == 0);

  }

  SECTION("message filter") {
    conn.output->clear();
    MessageFilter &mf = conn.message_filter;
    vector<PmMessage> messages;

    // by default sysex, bank CC, and program change are filtered out

    messages.push_back(Pm_Message(NOTE_ON, 64, 127));
    messages.push_back(Pm_Message(START, 0, 0));
    messages.push_back(Pm_Message(SYSEX, 1, 2));            // start of sysex
    messages.push_back(Pm_Message(3, CLOCK, EOX));          // realtime inside SYSEX
    messages.push_back(Pm_Message(CONTROLLER, 7, 127));     // volume CC
    messages.push_back(Pm_Message(CONTROLLER, CC_BANK_SELECT_MSB, 127)); // bank MSB
    messages.push_back(Pm_Message(PROGRAM_CHANGE, 2, 127));
    messages.push_back(Pm_Message(POLY_PRESSURE, 64, 0));
    messages.push_back(Pm_Message(CHANNEL_PRESSURE, 64, 0));
    messages.push_back(Pm_Message(NOTE_OFF, 64, 127));
    messages.push_back(Pm_Message(SYSTEM_RESET, 0, 0));

    SECTION("filter sysex") {
      REQUIRE(mf.sysex == false); // check default value
      REQUIRE(mf.program_change == false); // check default value

      for (auto msg : messages)
        conn.midi_in(msg);

      // Size is minus the two sysex messages, the bank MSB, and the prog
      // chg but plus the clock inside of it.
      int num_sent = conn.output->num_io_messages;
      REQUIRE(num_sent == messages.size() - 4 + 1);
      REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127));
      REQUIRE(conn.output->io_messages[2] == Pm_Message(CLOCK, 0, 0));
      REQUIRE(conn.output->io_messages[num_sent - 1] == messages.back());
    }

    SECTION("pass through sysex and program changes") {
      mf.sysex = true;
      mf.program_change = true;

      for (auto msg : messages)
        conn.midi_in(msg);

      int num_sent = conn.output->num_io_messages;
      REQUIRE(num_sent == messages.size());
      REQUIRE(conn.output->io_messages[0] == Pm_Message(NOTE_ON, 64, 127));
      REQUIRE(conn.output->io_messages[2] == Pm_Message(SYSEX, 1, 2));
      REQUIRE(conn.output->io_messages[3] == Pm_Message(3, CLOCK, EOX));
      REQUIRE(conn.output->io_messages[num_sent - 1] == messages.back());
    }

    SECTION("filter note on and off") {
      mf.sysex = true;
      mf.program_change = true;
      mf.note = false;

      for (auto msg : messages)
        conn.midi_in(msg);

      int num_sent = conn.output->num_io_messages;
      REQUIRE(num_sent == messages.size() - 2);
      REQUIRE(conn.output->io_messages[0] == Pm_Message(START, 0, 0));
      REQUIRE(conn.output->io_messages[num_sent - 2] == Pm_Message(CHANNEL_PRESSURE, 64, 0));
    }
  }
}

TEST_CASE("editing when not running", CATCH_CATEGORY) {
  Input in(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Output out(UNDEFINED_ID, pmNoDevice, "out 1 port", "out1");
  Connection conn(UNDEFINED_ID, &in, 0, &out, 1);

  REQUIRE(conn.xpose == 0);
  REQUIRE(!conn.is_running());
  conn.begin_changes();
  conn.xpose = 12;
  conn.end_changes();

  REQUIRE(!conn.is_running());
  REQUIRE(conn.xpose == 12);
  REQUIRE(conn.input == &in);
  REQUIRE(conn.output == &out);
}

TEST_CASE("editing when running and input changes", CATCH_CATEGORY) {
  Input in_old(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Input in_new(UNDEFINED_ID, pmNoDevice, "in 1 port", "in1");
  Output out(UNDEFINED_ID, pmNoDevice, "out 1 port", "out1");
  Connection conn(UNDEFINED_ID, &in_old, 0, &out, 1);

  conn.start();

  REQUIRE(conn.is_running());
  conn.begin_changes();
  REQUIRE(!conn.is_running());

  conn.xpose = 12;
  conn.input = &in_new;

  conn.end_changes();
  REQUIRE(conn.is_running());

  REQUIRE(conn.xpose == 12);
  REQUIRE(conn.input == &in_new);
  REQUIRE(conn.output == &out);
  REQUIRE(in_old.connections.empty());
  REQUIRE(in_new.connections.front() == &conn);

  conn.stop();
}
