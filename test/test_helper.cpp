#include <string>
#include <fstream>
#include <streambuf>
#include <unistd.h>
#include "test_helper.h"
#include "../src/storage.h"

void error_message(const char * const msg) {
  fprintf(stderr, "%s\n", msg);
}

void _initialize_database() {
  if (access(TEST_DB_PATH, F_OK) != -1)
    remove(TEST_DB_PATH);

  Storage storage(TEST_DB_PATH);
  storage.initialize();
  storage.close();
}

void _initialize_and_load_database() {
  _initialize_database();

  sqlite3 *db;
  int status = sqlite3_open(TEST_DB_PATH, &db);
  if (status != 0) {
    fprintf(stderr,  "error opening database file %s\n", TEST_DB_PATH);
    exit(1);
  }
  fprintf(stderr, "opened %s db %p in _initialize_and_load_database\n", TEST_DB_PATH, db); // DEBUG

  // read data file and execute
  char *error_buf;
  std::ifstream data_t(TEST_DATA_PATH);
  std::string data_sql((std::istreambuf_iterator<char>(data_t)),
                       std::istreambuf_iterator<char>());
  status = sqlite3_exec(db, data_sql.c_str(), nullptr, nullptr, &error_buf);
  if (status != 0) {
    fprintf(stderr, "error loading test data: %s\n", error_buf);
    fprintf(stderr, "closing in error db %p in _initialize_and_load_database with close_v2\n", db); // DEBUG
    if ((status = sqlite3_close_v2(db)) != 0)
      fprintf(stderr, "error closing database file %s\n", TEST_DB_PATH);
    sqlite3_free(error_buf);
    exit(1);
  }

  fprintf(stderr, "closing db %p in _initialize_and_load_database with close_v2\n", db); // DEBUG
  if ((status = sqlite3_close_v2(db)) != 0)
    fprintf(stderr, "error closing database file %s\n", TEST_DB_PATH);
}

KeyMaster *load_test_data() {
  KeyMaster *old_pm = KeyMaster_instance();
  if (old_pm)
    delete old_pm;

  _initialize_and_load_database();

  Storage storage(TEST_DB_PATH);
  KeyMaster *km = storage.load(true);
  storage.close();
  if (storage.has_error()) {
    fprintf(stderr, "load_test_data storage error: %s\n",
            storage.error().c_str());
    exit(1);
  }
  km->set_testing(true);
  return km;
}

Connection *create_conn() {
  Input *in = new Input(UNDEFINED_ID, pmNoDevice, "in1", "in 1 port");
  Output *out = new Output(UNDEFINED_ID, pmNoDevice, "out1", "out 1 port");
  Connection *conn = new Connection(UNDEFINED_ID, in, 0, out, 0);
  conn->start();                // add conn to input
  return conn;
}
