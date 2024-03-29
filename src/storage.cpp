#include <string>
#include <fstream>
#include <streambuf>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <err.h>
#include <portmidi.h>
#include <sqlite3.h>
#include "keymaster.h"
#include "consts.h"
#include "cursor.h"
#include "storage.h"
#include "formatter.h"
#include "schema.sql.h"
#include "device.h"

using namespace std;

#define SCHEMA_VERSION 1

Storage::Storage(const char *path) : loading_version(0) {
  int status = sqlite3_open(path, &db);
  if (status != 0) {
    db = nullptr;
    char error_buf[BUFSIZ];
    snprintf(error_buf, BUFSIZ, "error opening database file %s", path);
    error_str = error_buf;
  }
}

Storage::~Storage() {
  close();
  // sqlite3_prepare_v3() seems to set errno to 2 for no good reason that I
  // can tell. Reinitialize it here.
  errno = 0;
}

// Does not stop old km or start new km.
KeyMaster *Storage::load(bool testing) {
  KeyMaster *old_km = KeyMaster_instance();
  int status;

  if (db == nullptr)
    return old_km;

  km = new KeyMaster();    // side-effect: KeyMaster static instance set
  km->set_testing(testing);
  km->load_instruments();

  load_schema_version();
  load_instruments();
  load_velocity_curves();
  load_messages();
  load_triggers();
  load_songs();
  load_set_lists();
  create_default_patches();

  km->clear_modified();
  close();
  return km;
}

// Ignores all in-memory database object ids, generating new ones instead.
// Objects are written in "dependency order" so that all newly assigned ids
// are available for later writing.
void Storage::save(KeyMaster *keymaster, bool testing) {
  if (db == nullptr)
    return;

  initialize();
  if (has_error())
    return;

  km = keymaster;
  save_schema_version();
  save_instruments();
  save_velocity_curves();
  save_messages();
  save_triggers();
  save_songs();
  save_set_lists();

  km->clear_modified();
  close();
}

void Storage::close() {
  if (db != nullptr) {
    sqlite3_close_v2(db);
    db = nullptr;
  }
}

bool Storage::has_error() {
  return error_str != "";
}

string Storage::error() {
  return error_str;
}

// Initializes the database by dropping/recreating all tables and adding
// some data.
void Storage::initialize() {
  char *error_buf;

  // execute schema strings defined in schema.sql.h
  int status = sqlite3_exec(db, SCHEMA_SQL, nullptr, nullptr, &error_buf);
  if (status != 0) {
    fprintf(stderr, "error initializing database: %s\n", error_buf);
    error_str = error_buf;
    sqlite3_free(error_buf);
  }

  vector<Curve *> generated;
  generate_default_curves(generated);
  save_velocity_curves(generated);
}

int Storage::schema_version() {
  return SCHEMA_VERSION;
}

// ================================================================
// load helpers
// ================================================================

void Storage::load_schema_version() {
  sqlite3_stmt *stmt;
  const char * const sql = "select version from schema_version";
  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    loading_version = sqlite3_column_int(stmt, 1);
  }
  sqlite3_finalize(stmt);

  if (loading_version > SCHEMA_VERSION)
    fprintf(stderr, "warning: db schema version is v%d, but I only understand v%d\n",
            loading_version, SCHEMA_VERSION);
}

void Storage::load_instruments() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id, type, name, device_name from instruments";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    int type = sqlite3_column_int(stmt, 1);
    const char *name = (const char *)sqlite3_column_text(stmt, 2);
    const char *device_name = (const char *)sqlite3_column_text(stmt, 3);

    // Try to match device with what's been created in km already. If it
    // exists, update the db id and the name (not port name). If it does not
    // exist, create a new one (which will be disabled).
    PmDeviceID device_id = find_device(device_name, type);
    if (device_id != pmNoDevice) {
      bool found = false;

      // update db id and name
      if (type == INSTRUMENT_TYPE_INPUT) {
        for (auto &input : km->inputs()) {
          if (input->device_id == device_id && input->id() == UNDEFINED_ID) {
            input->set_id(id);
            input->set_name(name);
            found = true;
          }
        }
      }
      else {
        for (auto &output : km->outputs()) {
          if (output->device_id == device_id && output->id() == UNDEFINED_ID) {
            output->set_id(id);
            output->set_name(name);
            found = true;
          }
        }
      }
      if (!found)
        goto CREATE_NEW_INSTRUMENT;
    }
    else {
    CREATE_NEW_INSTRUMENT:
      if (type == INSTRUMENT_TYPE_INPUT)
        km->add_input(new Input(id, pmNoDevice, device_name, name));
      else
        km->add_output(new Output(id, pmNoDevice, device_name, name));
    }
  }
  sqlite3_finalize(stmt);
}

void Storage::load_velocity_curves() {
  sqlite3_stmt *stmt;
  const char * const sql = "select id, name, short_name, curve from velocity_curves";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int col = 0;
    sqlite3_int64 id = sqlite3_column_int64(stmt, col++);
    const char *name = text_or_null(stmt, col++, "");
    const char *short_name = text_or_null(stmt, col++, "");
    const char *bytes = (const char *)sqlite3_column_text(stmt, col++);

    Curve *curve = new Curve(id, name, short_name);
    km->add_velocity_curve(curve);
    curve->from_chars(bytes);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_messages() {
  sqlite3_stmt *stmt;
  const char * const sql = "select id, name, bytes from messages order by id";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    const char *name = text_or_null(stmt, 1, "");
    const char *bytes = (const char *)sqlite3_column_text(stmt, 2);

    Message *m = new Message(id, name);
    km->add_message(m);
    m->from_chars(bytes);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_triggers() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id, trigger_key_code, input_id, trigger_message_bytes,"
    "   action, message_id"
    " from triggers"
    " order by id";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    int trigger_key_code = int_or_null(stmt, 1);
    sqlite3_int64 input_id = id_or_null(stmt, 2);
    const char *bytes = (const char *)sqlite3_column_text(stmt, 3);
    const char *action_name = (const char *)sqlite3_column_text(stmt, 4);
    sqlite3_int64 message_id = id_or_null(stmt, 5);

    Message *output_message = nullptr;
    if (message_id != UNDEFINED_ID)
      output_message = find_message_by_id("trigger", id, message_id);

    TriggerAction action = TA_MESSAGE;
    if (action_name == nullptr)
      action = TA_MESSAGE;
    else if (strcmp(action_name, "next_song") == 0)
      action = TA_NEXT_SONG;
    else if (strcmp(action_name, "prev_song") == 0)
      action = TA_PREV_SONG;
    else if (strcmp(action_name, "next_patch") == 0)
      action = TA_NEXT_PATCH;
    else if (strcmp(action_name, "prev_patch") == 0)
      action = TA_PREV_PATCH;
    else if (strcmp(action_name, "panic") == 0)
      action = TA_PANIC;
    else if (strcmp(action_name, "super_panic") == 0)
      action = TA_SUPER_PANIC;
    else if (strcmp(action_name, "toggle_clock") == 0)
      action = TA_TOGGLE_CLOCK;

    Trigger *t = new Trigger(id, action, output_message);
    km->add_trigger(t);

    if (trigger_key_code != UNDEFINED)
      t->set_trigger_key_code(trigger_key_code);
    if (input_id != UNDEFINED_ID) {
      Input *input = find_input_by_id("trigger", id, input_id);
      PmMessage trigger_message = single_message_from_hex_bytes((char *)bytes);
      t->set_trigger_message(input, trigger_message);
    }
  }
  sqlite3_finalize(stmt);
}

void Storage::load_songs() {
  sqlite3_stmt *stmt;
  const char * const sql = "select id, name, notes, bpm, clock_on_at_start from songs order by name";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    const char *notes = (const char *)sqlite3_column_text(stmt, 2);
    float bpm = sqlite3_column_double(stmt, 3);
    int clock_on_at_start = sqlite3_column_int(stmt, 4);

    Song *s = new Song(id, name);
    if (notes != nullptr) s->set_notes(notes);
    s->set_bpm(bpm);
    s->set_clock_on_at_start(clock_on_at_start != 0);
    km->all_songs()->add_song(s);
    load_patches(s);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_patches(Song *s) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id, name, start_message_id, stop_message_id"
    " from patches"
    " where song_id = ?"
    " order by position";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int64(stmt, 1, s->id());
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    sqlite3_int64 start_message_id = id_or_null(stmt, 2);
    sqlite3_int64 stop_message_id = id_or_null(stmt, 3);

    Patch *p = new Patch(id, name);
    if (start_message_id != UNDEFINED_ID) {
      for (auto& message : km->messages()) {
        if (message->id() == start_message_id)
          p->set_start_message(message);
      }
      if (p->start_message() == nullptr) {
        char error_buf[BUFSIZ];
        snprintf(error_buf, BUFSIZ,
                "patch %lld (%s) can't find start message with id %lld\n",
                id, name, start_message_id);
        error_str = error_buf;
      }
    }
    if (stop_message_id != UNDEFINED_ID) {
      for (auto& message : km->messages()) {
        if (message->id() == stop_message_id)
          p->set_stop_message(message);
      }
      if (p->stop_message() == nullptr) {
        char error_buf[BUFSIZ];
        snprintf(error_buf, BUFSIZ,
                "patch %lld (%s) can't find stop message with id %lld\n",
                id, name, stop_message_id);
        error_str = error_buf;
      }
    }

    s->add_patch(p);
    load_connections(p);
  }

  sqlite3_finalize(stmt);
}

void Storage::create_default_patches() {
  for (auto& song : km->all_songs()->songs())
    if (song->patches().empty())
      create_default_patch(song);
}

// Connects first input to first output. Name of patch == song name.
void Storage::create_default_patch(Song *s) {
  Patch *p = new Patch(UNDEFINED_ID, s->name().c_str());
  s->add_patch(p);
  if (km->inputs().empty() || km->outputs().empty())
    return;
  Input *input = km->inputs().front();
  Output *output = km->outputs().front();
  Connection *conn =
    new Connection(UNDEFINED_ID, input, CONNECTION_ALL_CHANNELS,
                   output, CONNECTION_ALL_CHANNELS);
  p->add_connection(conn);
}

void Storage::load_connections(Patch *p) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id,"
    "   input_id, input_chan, output_id, output_chan,"
    "   bank_msb, bank_lsb, prog,"
    "   zone_low, zone_high, xpose, velocity_curve_id,"
    "   note, poly_pressure, chan_pressure, program_change, pitch_bend,"
    "   controller, song_pointer, song_select, tune_request, sysex,"
    "   clock, start_continue_stop, system_reset"
    " from connections"
    " where patch_id = ?"
    " order by position";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int64(stmt, 1, p->id());
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int col = 0;

    sqlite3_int64 id = sqlite3_column_int64(stmt, col++);
    sqlite3_int64 input_id = sqlite3_column_int64(stmt, col++);
    int input_chan = int_or_null(stmt, col++, CONNECTION_ALL_CHANNELS);
    sqlite3_int64 output_id = sqlite3_column_int64(stmt, col++);
    int output_chan = int_or_null(stmt, col++, CONNECTION_ALL_CHANNELS);
    int bank_msb = int_or_null(stmt, col++);
    int bank_lsb = int_or_null(stmt, col++);
    int prog = int_or_null(stmt, col++);
    int zone_low = int_or_null(stmt, col++, 0);
    int zone_high = int_or_null(stmt, col++, 127);
    int xpose = int_or_null(stmt, col++, 0);
    sqlite3_int64 velocity_curve_id = sqlite3_column_int64(stmt, col++);

    int note = sqlite3_column_int(stmt, col++);
    int poly_pressure = sqlite3_column_int(stmt, col++);
    int chan_pressure = sqlite3_column_int(stmt, col++);
    int program_change = sqlite3_column_int(stmt, col++);
    int pitch_bend = sqlite3_column_int(stmt, col++);
    int controller = sqlite3_column_int(stmt, col++);
    int song_pointer = sqlite3_column_int(stmt, col++);
    int song_select = sqlite3_column_int(stmt, col++);
    int tune_request = sqlite3_column_int(stmt, col++);
    int sysex = sqlite3_column_int(stmt, col++);
    int clock = sqlite3_column_int(stmt, col++);
    int start_continue_stop = sqlite3_column_int(stmt, col++);
    int system_reset = sqlite3_column_int(stmt, col++);

    Input *input = find_input_by_id("connection", id, input_id);
    Output *output = find_output_by_id("connection", id, output_id);
    Connection *conn = new Connection(id, input, input_chan, output, output_chan);
    MessageFilter &mf = conn->message_filter();

    conn->set_program_bank_msb(bank_msb);
    conn->set_program_bank_lsb(bank_lsb);
    conn->set_program_prog(prog);
    conn->set_zone_low(zone_low);
    conn->set_zone_high(zone_high);
    conn->set_xpose(xpose);
    conn->set_velocity_curve(km->velocity_curve_with_id(velocity_curve_id));
    mf.set_note(note);
    mf.set_poly_pressure(poly_pressure);
    mf.set_chan_pressure(chan_pressure);
    mf.set_program_change(program_change);
    mf.set_pitch_bend(pitch_bend);
    mf.set_controller(controller);
    mf.set_song_pointer(song_pointer);
    mf.set_song_select(song_select);
    mf.set_tune_request(tune_request);
    mf.set_sysex(sysex);
    mf.set_clock(clock);
    mf.set_start_continue_stop(start_continue_stop);
    mf.set_system_reset(system_reset);

    load_controller_mappings(conn);

    p->add_connection(conn);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_controller_mappings(Connection *conn) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id, cc_num, translated_cc_num, filtered,"
    "   pass_through_0, pass_through_127,"
    "   min_in, max_in, min_out, max_out"
    " from controller_mappings"
    " where connection_id = ?";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int64(stmt, 1, conn->id());
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    int cc_num = sqlite3_column_int(stmt, 1);
    int translated_cc_num = sqlite3_column_int(stmt, 2);
    int filtered_bool = sqlite3_column_int(stmt, 3);
    int pass_through_0 = sqlite3_column_int(stmt, 4);
    int pass_through_127 = sqlite3_column_int(stmt, 5);
    int min_in = sqlite3_column_int(stmt, 6);
    int max_in = sqlite3_column_int(stmt, 7);
    int min_out = sqlite3_column_int(stmt, 8);
    int max_out = sqlite3_column_int(stmt, 9);

    Controller *cc = new Controller(id, cc_num);
    cc->set_translated_cc_num(translated_cc_num);
    cc->set_filtered(filtered_bool != 0);
    cc->set_range(pass_through_0, pass_through_127,
                  min_in, max_in, min_out, max_out);
    conn->set_cc_map(cc->cc_num(), cc);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_set_lists() {
  sqlite3_stmt *stmt;
  const char * const sql = "select id, name from set_lists order by name";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    SetList *slist = new SetList(id, name);
    km->add_set_list(slist);
    load_set_list_songs(slist);
  }
  sqlite3_finalize(stmt);
}

void Storage::load_set_list_songs(SetList *slist) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select song_id"
    " from set_lists_songs"
    " where set_list_id = ?"
    " order by position";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int64(stmt, 1, slist->id());
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 song_id = sqlite3_column_int64(stmt, 0);
    Song *song = find_song_by_id("set list", slist->id(), song_id);
    slist->add_song(song);
  }
  sqlite3_finalize(stmt);
}

// ================================================================
// save helpers
// ================================================================

void Storage::save_schema_version() {
  sqlite3_stmt *stmt;
  const char * const sql = "insert into schema_version values (?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int(stmt, 1, SCHEMA_VERSION);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

void Storage::save_instruments() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into instruments (id, type, name, device_name) values (?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& input : km->inputs()) {
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_int(stmt, 2, 0);
    sqlite3_bind_text(stmt, 3, input->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, input->device_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    extract_id(input);
    sqlite3_reset(stmt);
  }
  for (auto& output : km->outputs()) {
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_int(stmt, 2, 1);
    sqlite3_bind_text(stmt, 3, output->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, output->device_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    extract_id(output);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_velocity_curves() {
  save_velocity_curves(km->velocity_curves());
}

void Storage::save_velocity_curves(vector<Curve *> &curves) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into velocity_curves (id, name, short_name, curve) values (?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& curve : curves) {
    char *hex_chars = bytes_to_hex(curve->curve, 128);

    int col = 1;
    sqlite3_bind_null(stmt, col++);
    sqlite3_bind_text(stmt, col++, curve->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, col++, curve->short_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, col++, hex_chars, -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    extract_id(curve);
    sqlite3_reset(stmt);

    free(hex_chars);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_messages() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into messages (id, name, bytes) values (?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& msg : km->messages()) {
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_text(stmt, 2, msg->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, msg->to_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    extract_id(msg);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_triggers() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into triggers"
    "   (id, trigger_key_code, input_id, trigger_message_bytes, action, message_id)"
    " values"
    "   (?, ?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& trigger : km->triggers()) {
    Input *input = trigger->input();

    sqlite3_bind_null(stmt, 1);
    bind_int_or_null(stmt, 2, trigger->trigger_key_code());
    bind_obj_id_or_null(stmt, 3, input);
    if (trigger->trigger_message() == Pm_Message(0, 0, 0))
      sqlite3_bind_null(stmt, 4);
    else
      sqlite3_bind_text(stmt, 4,
                        single_message_to_hex_bytes(trigger->trigger_message()).c_str(),
                        -1, SQLITE_TRANSIENT);
    if (trigger->output_message() != nullptr) {
      sqlite3_bind_null(stmt, 5);
      sqlite3_bind_int64(stmt, 6, trigger->output_message()->id());
    }
    else {
      const char * action;
      switch (trigger->action()) {
      case TA_NEXT_SONG: action = "next_song"; break;
      case TA_PREV_SONG: action = "prev_song"; break;
      case TA_NEXT_PATCH: action = "next_patch"; break;
      case TA_PREV_PATCH: action = "prev_patch"; break;
      case TA_PANIC: action = "panic"; break;
      case TA_SUPER_PANIC: action = "super_panic"; break;
      case TA_TOGGLE_CLOCK: action = "toggle_clock"; break;
      default: break;
      }
      sqlite3_bind_text(stmt, 5, action, -1, SQLITE_TRANSIENT);
      sqlite3_bind_null(stmt, 6);
    }
    sqlite3_step(stmt);
    extract_id(trigger);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_songs() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into songs (id, name, notes, bpm, clock_on_at_start) values (?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& song : km->all_songs()->songs()) {
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_text(stmt, 2, song->name().c_str(), -1, SQLITE_TRANSIENT);
    if (song->notes().empty())
      sqlite3_bind_null(stmt, 3);
    else
      sqlite3_bind_text(stmt, 3, song->notes().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, song->bpm());
    sqlite3_bind_int(stmt, 5, song->clock_on_at_start() ? 1 : 0);
    sqlite3_step(stmt);
    extract_id(song);
    sqlite3_reset(stmt);
    save_patches(song);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_patches(Song *song) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into patches"
    "   (id, song_id, position, name, start_message_id, stop_message_id)"
    " values (?, ?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  int position = 0;
  for (auto& patch : song->patches()) {
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_int64(stmt, 2, song->id());
    sqlite3_bind_int(stmt, 3, position++);
    sqlite3_bind_text(stmt, 4, patch->name().c_str(), -1, SQLITE_TRANSIENT);
    bind_obj_id_or_null(stmt, 5, patch->start_message());
    bind_obj_id_or_null(stmt, 6, patch->stop_message());
    sqlite3_step(stmt);
    extract_id(patch);
    sqlite3_reset(stmt);
    save_connections(patch);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_connections(Patch *patch) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into connections"
    "   (id, patch_id, position, input_id, input_chan, output_id, output_chan,"
    "    bank_msb, bank_lsb, prog, zone_low, zone_high, xpose, velocity_curve_id,"
    "    note, poly_pressure, chan_pressure, program_change, pitch_bend,"
    "    controller, song_pointer, song_select, tune_request, sysex,"
    "    clock, start_continue_stop, system_reset)"
    " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  int position = 0;
  for (auto& conn : patch->connections()) {
    int col = 1;
    sqlite3_bind_null(stmt, col++);
    sqlite3_bind_int64(stmt, col++, patch->id());
    sqlite3_bind_int(stmt, col++, position++);
    sqlite3_bind_int64(stmt, col++, conn->input()->id());
    bind_int_or_null(stmt, col++, conn->input_chan(), CONNECTION_ALL_CHANNELS);
    sqlite3_bind_int64(stmt, col++, conn->output()->id());
    bind_int_or_null(stmt, col++, conn->output_chan(), CONNECTION_ALL_CHANNELS);
    bind_int_or_null(stmt, col++, conn->program_bank_msb());
    bind_int_or_null(stmt, col++, conn->program_bank_lsb());
    bind_int_or_null(stmt, col++, conn->program_prog());
    sqlite3_bind_int(stmt, col++, conn->zone_low());
    sqlite3_bind_int(stmt, col++, conn->zone_high());
    sqlite3_bind_int(stmt, col++, conn->xpose());
    bind_obj_id_or_null(stmt, col++, conn->velocity_curve());

    MessageFilter &mf = conn->message_filter();
    sqlite3_bind_int(stmt, col++, mf.note() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.poly_pressure() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.chan_pressure() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.program_change() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.pitch_bend() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.controller() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.song_pointer() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.song_select() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.tune_request() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.sysex() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.clock() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.start_continue_stop() ? 1 : 0);
    sqlite3_bind_int(stmt, col++, mf.system_reset() ? 1 : 0);

    sqlite3_step(stmt);
    extract_id(conn);
    sqlite3_reset(stmt);
    save_controller_mappings(conn);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_controller_mappings(Connection *conn) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into controller_mappings"
    "   (id, connection_id, cc_num, translated_cc_num, filtered,"
    "    pass_through_0, pass_through_127, min_in, max_in, min_out, max_out)"
    " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (int i = 0; i < 128; ++i) {
    Controller *cc = conn->cc_map(i);
    if (cc == nullptr)
      continue;

    int j = 1;
    sqlite3_bind_null(stmt, j++);
    sqlite3_bind_int64(stmt, j++, conn->id());
    sqlite3_bind_int(stmt, j++, cc->cc_num());
    sqlite3_bind_int(stmt, j++, cc->translated_cc_num());
    sqlite3_bind_int(stmt, j++, cc->filtered() ? 1 : 0);
    sqlite3_bind_int(stmt, j++, cc->pass_through_0() ? 1 : 0);
    sqlite3_bind_int(stmt, j++, cc->pass_through_127() ? 1 : 0);
    sqlite3_bind_int(stmt, j++, cc->min_in());
    sqlite3_bind_int(stmt, j++, cc->max_in());
    sqlite3_bind_int(stmt, j++, cc->min_out());
    sqlite3_bind_int(stmt, j++, cc->max_out());
    sqlite3_step(stmt);
    extract_id(cc);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_set_lists() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into set_lists (id, name) values (?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto &set_list : km->set_lists()) {
    if (set_list == km->all_songs())
      continue;
    sqlite3_bind_null(stmt, 1);
    sqlite3_bind_text(stmt, 2, set_list->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    extract_id(set_list);
    sqlite3_reset(stmt);
    save_set_list_songs(set_list);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_set_list_songs(SetList *set_list) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into set_lists_songs (set_list_id, song_id, position)"
    " values (?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  int position = 0;
  for (auto &song : set_list->songs()) {
    sqlite3_bind_int64(stmt, 1, set_list->id());
    sqlite3_bind_int64(stmt, 2, song->id());
    sqlite3_bind_int(stmt, 3, position++);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

// ================================================================
// find by id
// ================================================================

Input *Storage::find_input_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &input : km->inputs())
    if (input->id() == id)
      return input;
  set_find_error_message(searcher_name, searcher_id, "input", id);
  return nullptr;
}

Output *Storage::find_output_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &output : km->outputs())
    if (output->id() == id)
      return output;
  set_find_error_message(searcher_name, searcher_id, "output", id);
  return nullptr;
}

Message *Storage::find_message_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &msg : km->messages())
    if (msg->id() == id)
      return msg;
  set_find_error_message(searcher_name, searcher_id, "message", id);
  return nullptr;
}

Song *Storage::find_song_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &song : km->all_songs()->songs())
    if (song->id() == id)
        return song;
  set_find_error_message(searcher_name, searcher_id, "song", id);
  return nullptr;
}

void Storage::set_find_error_message(
  const char * const searcher_name, sqlite3_int64 searcher_id,
  const char * const find_name, sqlite3_int64 find_id
) {
  char error_buf[BUFSIZ];
  snprintf(error_buf, BUFSIZ, "%s (%lld) can't find %s with id %lld\n",
          searcher_name, searcher_id, find_name, find_id);
  error_str = error_buf;
}

// ================================================================

int Storage::int_or_null(sqlite3_stmt *stmt, int col_num, int null_val) {
  return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
    ? null_val
    : sqlite3_column_int(stmt, col_num);
}

sqlite3_int64 Storage::id_or_null(sqlite3_stmt *stmt, int col_num, sqlite3_int64 null_val) {
  return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
    ? null_val
    : sqlite3_column_int64(stmt, col_num);
}

const char *Storage::text_or_null(sqlite3_stmt *stmt, int col_num, const char *null_val) {
  return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
    ? null_val
    : (const char *)sqlite3_column_text(stmt, col_num);
}

void Storage::bind_obj_id_or_null(sqlite3_stmt *stmt, int col_num, DBObj *obj_ptr) {
  if (obj_ptr == nullptr || obj_ptr->id() == UNDEFINED_ID)
    sqlite3_bind_null(stmt, col_num);
  else
    sqlite3_bind_int64(stmt, col_num, obj_ptr->id());
}

void Storage::bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval) {
  if (val == nullval)
    sqlite3_bind_null(stmt, col_num);
  else
    sqlite3_bind_int(stmt, col_num, val);
}

void Storage::extract_id(DBObj *db_obj) {
  db_obj->set_id(sqlite3_last_insert_rowid(db));
}

// FIXME use standard Message output format 007f35b0
//
// Parses a single array of chars of length <= 6 representing a single
// non-separated MIDI messages like 'b0357f'. Returns a PmMessage. If
// `bytes` is null, returns a message containing all zeroes.
PmMessage Storage::single_message_from_hex_bytes(char *bytes) {
  if (bytes == nullptr)
    return Pm_Message(0, 0, 0);

  // FIXME same as inner loop of messages_from_chars in message.cpp
  unsigned char byte;
  PmMessage msg = 0;

  for (int i = 0; i < 4; ++i) {
    msg <<= 8;
    msg += hex_to_byte(bytes);
    bytes += 2;
  }

  return msg;
}

// FIXME use standard Message format 007f35b0
string Storage::single_message_to_hex_bytes(PmMessage msg) {
  char buf[16], *p = buf;

  for (int i = 0; i < 4; ++i) {
    snprintf(p, 3, "%02x", (unsigned char)(msg & 0xff));
    msg >>= 8;
    p += 2;
  }
  return string(buf);
}
