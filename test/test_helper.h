#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <portmidi.h>
#include "../src/consts.h"
#include "../src/connection.h"
#include "../src/keymaster.h"
#include "../src/input.h"
#include "../src/output.h"
#include "../src/cursor.h"

#define LOCAL_SCHEMA_PATH "db/schema.sql"
#define TEST_DATA_PATH "test/testdata.sql"
#define TEST_DB_PATH "/tmp/keymaster_test.km"

KeyMaster *load_test_data();
Connection *create_conn();

#endif /* TEST_HELPER_H */
