#include "catch.hpp"
#include "test_helper.h"

#define CATCH_CATEGORY "[input]"

const char *BAD_INPUT_COUNT = "bad num input messages";
const char *BAD_OUTPUT_COUNT = "bad num output messages";
const char *BAD_INPUT = "wrong input recorded";
const char *BAD_OUTPUT = "wrong output recorded";

PmMessage *test_events() {
  PmMessage *buf = (PmMessage *)malloc(4 * sizeof(PmMessage));
  buf[0] = Pm_Message(NOTE_ON, 64, 127);           // note on
  buf[1] = Pm_Message(CONTROLLER, CC_VOLUME, 127); // volume
  buf[2] = Pm_Message(NOTE_OFF, 64, 127);          // note off
  buf[3] = Pm_Message(TUNE_REQUEST, 0, 0);         // tune request
  return buf;
}

TEST_CASE("through connection", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;
  PmMessage *buf = test_events();

  for (int i = 0; i < 4; ++i)
    in->read(buf[i]);

  REQUIRE(in->num_io_messages == 4);
  REQUIRE(out->num_io_messages == 4);
  for (int i = 0; i < 4; ++i) {
    REQUIRE(in->io_messages[i] == buf[i]);
    REQUIRE(out->io_messages[i] == buf[i]);
  }

  free(buf);
  delete conn;
}

TEST_CASE("two connections", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;

  Output *out2 = new Output(UNDEFINED_ID, "out2", "out2 port name", CONNECTION_ALL_CHANNELS);
  Connection *conn2 = new Connection(UNDEFINED_ID, in, 0, out2, 0);
  conn2->start();

  PmMessage *buf = test_events();
  for (int i = 0; i < 4; ++i)
    in->read(buf[i]);

  REQUIRE(in->num_io_messages == 4);
  REQUIRE(out->num_io_messages == 4);
  REQUIRE(out2->num_io_messages == 4);
  for (int i = 0; i < 4; ++i) {
    REQUIRE(in->io_messages[i] == buf[i]);
    REQUIRE(out->io_messages[i] == buf[i]);
    REQUIRE(out2->io_messages[i] == buf[i]);
  }

  free(buf);
  delete conn;
}

TEST_CASE("connection switch routes note offs correctly", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;

  Output *out2 = new Output(UNDEFINED_ID, "out2", "out2 port name", CONNECTION_ALL_CHANNELS);
  Connection *conn2 = new Connection(UNDEFINED_ID, in, 0, out2, 0);

  PmMessage *buf = test_events();

  for (int i = 0; i < 2; ++i)
    in->read(buf[i]);           // note on, volume
  conn->stop();
  conn2->start();
  for (int i = 2; i < 4; ++i)
    in->read(buf[i]);           // note off, tune request
  conn2->stop();

  // Make sure input sent all four messages
  REQUIRE(in->num_io_messages == 4);
  for (int i = 0; i < 4; ++i)
    REQUIRE(in->io_messages[i] == buf[i]);

  // Make sure out got the note on, volume, and note off
  REQUIRE(out->num_io_messages == 3);
  for (int i = 0; i < 3; ++i)
    REQUIRE(out->io_messages[i] == buf[i]);

  // Make sure out2 got the tune request
  REQUIRE(out2->num_io_messages == 1);
  REQUIRE(out2->io_messages[0] == buf[3]); // out2 got tune request

  free(buf);
  delete conn2;
  delete conn;
}

TEST_CASE("connection switch pays attention to note off channel", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;

  Output *out2 = new Output(UNDEFINED_ID, "out2", "out2 port name", CONNECTION_ALL_CHANNELS);
  Connection *conn2 = new Connection(UNDEFINED_ID, in, CONNECTION_ALL_CHANNELS,
                                     out2, CONNECTION_ALL_CHANNELS);

  PmMessage *buf = test_events();
  buf[2] = Pm_Message(NOTE_OFF + 3, 64, 127); // note off, different channel

  for (int i = 0; i < 2; ++i)
    in->read(buf[i]);           // note on, volume
  conn->stop();
  conn2->start();
  for (int i = 2; i < 4; ++i)
    in->read(buf[i]);           // note off (diff channel), tune request
  conn2->stop();

  // Make sure input sent all four messages
  REQUIRE(in->num_io_messages == 4);
  for (int i = 0; i < 4; ++i)
    REQUIRE(in->io_messages[i] == buf[i]);

  // Make sure out got the note on, volume, but not the note off
  REQUIRE(out->num_io_messages == 2);
  REQUIRE(out->io_messages[0] == buf[0]);
  REQUIRE(out->io_messages[1] == buf[1]);

  // Make sure out2 got the note off and the tune request
  REQUIRE(out2->num_io_messages == 2);
  REQUIRE(out2->io_messages[0] == buf[2]);
  REQUIRE(out2->io_messages[1] == buf[3]);

  free(buf);
  delete conn2;
  delete conn;
}

TEST_CASE("connection switch routes sustains correctly", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;

  Output *out2 = new Output(UNDEFINED_ID, "out2", "out2 port name", CONNECTION_ALL_CHANNELS);
  Connection *conn2 = new Connection(UNDEFINED_ID, in, 0, out2, 0);

  PmMessage buf[4] = {
    Pm_Message(NOTE_ON, 64, 127), // note on
    Pm_Message(CONTROLLER, CC_SUSTAIN, 127), // sustain on
    Pm_Message(NOTE_OFF, 64, 127),           // note off
    Pm_Message(CONTROLLER, CC_SUSTAIN, 0)    // sustain off
  };

  for (int i = 0; i < 2; ++i)
    in->read(buf[i]);           // note on, sustain on
  conn->stop();
  conn2->start();
  for (int i = 2; i < 4; ++i)
    in->read(buf[i]);           // note off, sustain off
  conn2->stop();

  // Make sure note off was sent to original output
  REQUIRE(in->num_io_messages == 4);
  REQUIRE(out->num_io_messages == 4);
  REQUIRE(out2->num_io_messages == 0);
  for (int i = 0; i < 4; ++i)
    REQUIRE(in->io_messages[i] == buf[i]);

  for (int i = 0; i < 4; ++i)
    REQUIRE(out->io_messages[i] == buf[i]);

  delete conn2;
  delete conn;
}

TEST_CASE("connection switch pays attention to sustain channel", CATCH_CATEGORY) {
  Connection *conn = create_conn();
  Input *in = conn->input;
  Output *out = conn->output;

  Output *out2 = new Output(UNDEFINED_ID, "out2", "out2 port name", CONNECTION_ALL_CHANNELS);
  Connection *conn2 = new Connection(UNDEFINED_ID, in, CONNECTION_ALL_CHANNELS,
                                     out2, CONNECTION_ALL_CHANNELS);

  PmMessage buf[4] = {
    Pm_Message(NOTE_ON, 64, 127),            // note on
    Pm_Message(CONTROLLER, CC_SUSTAIN, 127), // sustain on
    Pm_Message(NOTE_OFF, 64, 127),           // note off
    Pm_Message(CONTROLLER + 3, CC_SUSTAIN, 0) // sustain off, different channel
  };

  for (int i = 0; i < 2; ++i)
    in->read(buf[i]);           // note on, sustain on
  conn->stop();
  conn2->start();
  for (int i = 2; i < 4; ++i)
    in->read(buf[i]);           // note off, sustain off
  conn2->stop();

  REQUIRE(out->num_io_messages == 3);
  REQUIRE(out->io_messages[0] == buf[0]);
  REQUIRE(out->io_messages[1] == buf[1]);
  REQUIRE(out->io_messages[2] == buf[2]);

  REQUIRE(out2->num_io_messages == 1);
  REQUIRE(out2->io_messages[0] == buf[3]);

  delete conn2;
  delete conn;
}
