#include "catch.hpp"
#include "test_helper.h"

#define CATCH_CATEGORY "[input]"

PmMessage CONNECTION_TEST_EVENTS[] = {
  Pm_Message(NOTE_ON, 64, 127),           // note on
  Pm_Message(CONTROLLER, CC_VOLUME, 127), // volume
  Pm_Message(NOTE_OFF, 64, 127),          // note off
  Pm_Message(TUNE_REQUEST, 0, 0)          // tune request
};

TEST_CASE("connection filtering", CATCH_CATEGORY) {
  Input in(UNDEFINED_ID, pmNoDevice, "in1", "in 1 port");
  Output out(UNDEFINED_ID, pmNoDevice, "out1", "out 1 port");
  Output out2(UNDEFINED_ID, pmNoDevice, "out2", "out 2 port");
  Connection conn(UNDEFINED_ID, &in, 0, &out, 0);
  conn.start();

  SECTION("through connection") {
    for (int i = 0; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);

    REQUIRE(in.num_io_messages == 4);
    REQUIRE(out.num_io_messages == 4);
    for (int i = 0; i < 4; ++i) {
      REQUIRE(in.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
      REQUIRE(out.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
    }

  }

  SECTION("two connections") {
    Connection conn2(UNDEFINED_ID, &in, 0, &out2, 0);
    conn2.start();

    for (int i = 0; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);

    REQUIRE(in.num_io_messages == 4);
    REQUIRE(out.num_io_messages == 4);
    REQUIRE(out2.num_io_messages == 4);
    for (int i = 0; i < 4; ++i) {
      REQUIRE(in.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
      REQUIRE(out.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
      REQUIRE(out2.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
    }
  }

  SECTION("connection switch routes note offs correctly") {
    Connection conn2(UNDEFINED_ID, &in, 0, &out2, 0);

    for (int i = 0; i < 2; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note on, volume
    conn.stop();
    conn2.start();
    for (int i = 2; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note off, tune request
    conn2.stop();

    // Make sure input sent all four messages
    REQUIRE(in.num_io_messages == 4);
    for (int i = 0; i < 4; ++i)
      REQUIRE(in.io_messages[i] == CONNECTION_TEST_EVENTS[i]);

    // Make sure out got the note on, volume, and note off
    REQUIRE(out.num_io_messages == 3);
    for (int i = 0; i < 3; ++i)
      REQUIRE(out.io_messages[i] == CONNECTION_TEST_EVENTS[i]);

    // Make sure out2 got the tune request
    REQUIRE(out2.num_io_messages == 1);
    REQUIRE(out2.io_messages[0] == CONNECTION_TEST_EVENTS[3]); // out2 got tune request
  }

  SECTION("connection switch pays attention to note off channel") {
    Connection conn2(UNDEFINED_ID, &in, CONNECTION_ALL_CHANNELS, &out2, CONNECTION_ALL_CHANNELS);

    CONNECTION_TEST_EVENTS[2] = Pm_Message(NOTE_OFF + 3, 64, 127); // note off, different channel

    for (int i = 0; i < 2; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note on, volume
    conn.stop();
    conn2.start();
    for (int i = 2; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note off (diff channel), tune request
    conn2.stop();

    // Make sure input sent all four messages
    REQUIRE(in.num_io_messages == 4);
    for (int i = 0; i < 4; ++i)
      REQUIRE(in.io_messages[i] == CONNECTION_TEST_EVENTS[i]);

    // Make sure out got the note on, volume, but not the note off
    REQUIRE(out.num_io_messages == 2);
    REQUIRE(out.io_messages[0] == CONNECTION_TEST_EVENTS[0]);
    REQUIRE(out.io_messages[1] == CONNECTION_TEST_EVENTS[1]);

    // Make sure out2 got the note off and the tune request
    REQUIRE(out2.num_io_messages == 2);
    REQUIRE(out2.io_messages[0] == CONNECTION_TEST_EVENTS[2]);
    REQUIRE(out2.io_messages[1] == CONNECTION_TEST_EVENTS[3]);
  }

  SECTION("connection switch routes sustains correctly") {
    Connection conn2(UNDEFINED_ID, &in, 0, &out2, 0);

    PmMessage CONNECTION_TEST_EVENTS[4] = {
      Pm_Message(NOTE_ON, 64, 127), // note on
      Pm_Message(CONTROLLER, CC_SUSTAIN, 127), // sustain on
      Pm_Message(NOTE_OFF, 64, 127),           // note off
      Pm_Message(CONTROLLER, CC_SUSTAIN, 0)    // sustain off
    };

    for (int i = 0; i < 2; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note on, sustain on
    conn.stop();
    conn2.start();
    for (int i = 2; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note off, sustain off
    conn2.stop();

    // Make sure note off was sent to original output
    REQUIRE(in.num_io_messages == 4);
    REQUIRE(out.num_io_messages == 4);
    REQUIRE(out2.num_io_messages == 0);
    for (int i = 0; i < 4; ++i)
      REQUIRE(in.io_messages[i] == CONNECTION_TEST_EVENTS[i]);

    for (int i = 0; i < 4; ++i)
      REQUIRE(out.io_messages[i] == CONNECTION_TEST_EVENTS[i]);
  }

  SECTION("connection switch pays attention to sustain channel") {
    Connection conn2(UNDEFINED_ID, &in, CONNECTION_ALL_CHANNELS, &out2, CONNECTION_ALL_CHANNELS);

    PmMessage CONNECTION_TEST_EVENTS[4] = {
      Pm_Message(NOTE_ON, 64, 127),            // note on
      Pm_Message(CONTROLLER, CC_SUSTAIN, 127), // sustain on
      Pm_Message(NOTE_OFF, 64, 127),           // note off
      Pm_Message(CONTROLLER + 3, CC_SUSTAIN, 0) // sustain off, different channel
    };

    for (int i = 0; i < 2; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note on, sustain on
    conn.stop();
    conn2.start();
    for (int i = 2; i < 4; ++i)
      in.read(CONNECTION_TEST_EVENTS[i]);           // note off, sustain off
    conn2.stop();

    REQUIRE(out.num_io_messages == 3);
    REQUIRE(out.io_messages[0] == CONNECTION_TEST_EVENTS[0]);
    REQUIRE(out.io_messages[1] == CONNECTION_TEST_EVENTS[1]);
    REQUIRE(out.io_messages[2] == CONNECTION_TEST_EVENTS[2]);

    REQUIRE(out2.num_io_messages == 1);
    REQUIRE(out2.io_messages[0] == CONNECTION_TEST_EVENTS[3]);
  }
}
