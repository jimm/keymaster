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
#include "cursor.h"
#include "storage.h"
#include "formatter.h"
#include "schema.sql.h"
#include "device.h"

using namespace std;

#define INSTRUMENT_TYPE_INPUT 0
#define INSTRUMENT_TYPE_OUTPUT 1

Storage::Storage(const char *path) {
  int status = sqlite3_open(path, &db);
  if (status != 0) {
    db = nullptr;
    char error_buf[BUFSIZ];
    sprintf(error_buf,  "error opening database file %s", path);
    error_str = error_buf;
  }
}

Storage::~Storage() {
  if (db != nullptr)
    sqlite3_close(db);
}

// Does not stop old km or start new km.
KeyMaster *Storage::load(bool testing) {
  KeyMaster *old_km = KeyMaster_instance();
  int status;

  if (db == nullptr)
    return old_km;

  km = new KeyMaster();    // side-effect: KeyMaster static instance set
  km->testing = testing;
  km->load_instruments();

  load_instruments();
  load_messages();
  load_triggers();
  load_songs();
  load_set_lists();
  create_default_patches();

  return km;
}

void Storage::save(KeyMaster *keymaster, bool testing) {
  if (db == nullptr)
    return;

  initialize();
  if (has_error())
    return;

  km = keymaster;
  save_instruments();
  save_messages();
  save_triggers();
  save_songs();
  save_set_lists();
}

bool Storage::has_error() {
  return error_str != "";
}

string Storage::error() {
  return error_str;
}

void Storage::initialize() {
  char *error_buf;

  // execute schema strings defined in schema.sql.h
  int status = sqlite3_exec(db, SCHEMA_SQL, nullptr, nullptr, &error_buf);
  if (status != 0) {
    fprintf(stderr, "%s\n", error_buf);
    error_str = error_buf;
  }
}

// ================================================================
// load helpers
// ================================================================

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
        for (auto &input : km->inputs) {
          if (input->device_id == device_id && input->id() == UNDEFINED_ID) {
            input->set_id(id);
            input->name = name;
            found = true;
          }
        }
      }
      else {
        for (auto &output : km->outputs) {
          if (output->device_id == device_id && output->id() == UNDEFINED_ID) {
            output->set_id(id);
            output->name = name;
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
        km->inputs.push_back(new Input(id, pmNoDevice, device_name, name));
      else
        km->outputs.push_back(new Output(id, pmNoDevice, device_name, name));
    }
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
    km->messages.push_back(m);
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

    Trigger *t = new Trigger(id, action, output_message);
    km->triggers.push_back(t);

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
    int bpm = sqlite3_column_int(stmt, 3);
    int clock_on_at_start = sqlite3_column_int(stmt, 4);

    Song *s = new Song(id, name);
    if (notes != nullptr) s->notes = notes;
    s->bpm = bpm;
    s->clock_on_at_start = clock_on_at_start != 0;
    km->all_songs->songs.push_back(s);
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
      for (auto& message : km->messages) {
        if (message->id() == start_message_id)
          p->start_message = message;
      }
      if (p->start_message == nullptr) {
        char error_buf[BUFSIZ];
        sprintf(error_buf,
                "patch %lld (%s) can't find start message with id %lld\n",
                id, name, start_message_id);
        error_str = error_buf;
      }
    }
    if (stop_message_id != UNDEFINED_ID) {
      for (auto& message : km->messages) {
        if (message->id() == stop_message_id)
          p->stop_message = message;
      }
      if (p->stop_message == nullptr) {
        char error_buf[BUFSIZ];
        sprintf(error_buf,
                "patch %lld (%s) can't find stop message with id %lld\n",
                id, name, stop_message_id);
        error_str = error_buf;
      }
    }

    s->patches.push_back(p);
    load_connections(p);
  }

  sqlite3_finalize(stmt);
}

void Storage::create_default_patches() {
  for (auto& song : km->all_songs->songs)
    if (song->patches.empty())
      create_default_patch(song);
}

// This only works if input and output port names are the same. That's not
// true for many keyboards such as the Korg Kronos and the Studiologic
// Sledge.
void Storage::create_default_patch(Song *s) {
  Patch *p = new Patch(UNDEFINED_ID, "Default Patch");
  s->patches.push_back(p);
  for (auto& input : km->inputs) {
    for (auto& output : km->outputs) {
      if (output->device_name == input->device_name) {
        Connection *conn =
          new Connection(UNDEFINED_ID, input, CONNECTION_ALL_CHANNELS,
                         output, CONNECTION_ALL_CHANNELS);
        p->connections.push_back(conn);
      }
    }
  }
}

void Storage::load_connections(Patch *p) {
  sqlite3_stmt *stmt;
  const char * const sql =
    "select id,"
    "   input_id, input_chan, output_id, output_chan,"
    "   bank_msb, bank_lsb, prog,"
    "   zone_low, zone_high, xpose, pass_through_sysex"
    " from connections"
    " where patch_id = ?"
    " order by position";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  sqlite3_bind_int64(stmt, 1, p->id());
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    sqlite3_int64 id = sqlite3_column_int64(stmt, 0);
    sqlite3_int64 input_id = sqlite3_column_int64(stmt, 1);
    int input_chan = int_or_null(stmt, 2, CONNECTION_ALL_CHANNELS);
    sqlite3_int64 output_id = sqlite3_column_int64(stmt, 3);
    int output_chan = int_or_null(stmt, 4, CONNECTION_ALL_CHANNELS);
    int bank_msb = int_or_null(stmt, 5);
    int bank_lsb = int_or_null(stmt, 6);
    int prog = int_or_null(stmt, 7);
    int zone_low = int_or_null(stmt, 8, 0);
    int zone_high = int_or_null(stmt, 9, 127);
    int xpose = int_or_null(stmt, 10, 0);
    int pass_through_sysex_bool = sqlite3_column_int(stmt, 11);

    Input *input = find_input_by_id("connection", id, input_id);
    Output *output = find_output_by_id("connection", id, output_id);
    Connection *conn = new Connection(id, input, input_chan, output, output_chan);
    conn->prog.bank_msb = bank_msb;
    conn->prog.bank_lsb = bank_lsb;
    conn->prog.prog = prog;
    conn->zone.low = zone_low;
    conn->zone.high = zone_high;
    conn->xpose = xpose;
    conn->pass_through_sysex = pass_through_sysex_bool != 0;

    load_controller_mappings(conn);

    p->connections.push_back(conn);
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
    cc->translated_cc_num = translated_cc_num;
    cc->filtered = filtered_bool != 0;
    cc->set_range(pass_through_0, pass_through_127,
                  min_in, max_in, min_out, max_out);
    conn->cc_maps[cc->cc_num] = cc;
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
    km->set_lists.push_back(slist);
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
    slist->songs.push_back(song);
  }
  sqlite3_finalize(stmt);
}

// ================================================================
// save helpers
// ================================================================

void Storage::save_instruments() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into instruments (id, type, name, device_name) values (?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& input : km->inputs) {
    bind_obj_id_or_null(stmt, 1, input);
    sqlite3_bind_int(stmt, 2, 0);
    sqlite3_bind_text(stmt, 3, input->name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, input->device_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    extract_id(input);
    sqlite3_reset(stmt);
  }
  for (auto& output : km->outputs) {
    bind_obj_id_or_null(stmt, 1, output);
    sqlite3_bind_int(stmt, 2, 1);
    sqlite3_bind_text(stmt, 3, output->name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, output->device_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    extract_id(output);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);
}

void Storage::save_messages() {
  sqlite3_stmt *stmt;
  const char * const sql =
    "insert into messages (id, name, bytes) values (?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  for (auto& msg : km->messages) {
    bind_obj_id_or_null(stmt, 1, msg);
    sqlite3_bind_text(stmt, 2, msg->name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, msg->to_string().c_str(), -1, SQLITE_STATIC);
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
  for (auto& trigger : km->triggers) {
    Input *input = trigger->input();

    bind_obj_id_or_null(stmt, 1, trigger);
    bind_int_or_null(stmt, 2, trigger->trigger_key_code);
    bind_obj_id_or_null(stmt, 3, input);
    if (trigger->trigger_message == Pm_Message(0, 0, 0))
      sqlite3_bind_null(stmt, 4);
    else
      sqlite3_bind_text(stmt, 4,
                        single_message_to_hex_bytes(trigger->trigger_message).c_str(),
                        -1, SQLITE_STATIC);
    if (trigger->output_message != nullptr) {
      sqlite3_bind_null(stmt, 5);
      sqlite3_bind_int64(stmt, 6, trigger->output_message->id());
    }
    else {
      const char * action;
      switch (trigger->action) {
      case TA_NEXT_SONG: action = "next_song"; break;
      case TA_PREV_SONG: action = "prev_song"; break;
      case TA_NEXT_PATCH: action = "next_patch"; break;
      case TA_PREV_PATCH: action = "prev_patch"; break;
      case TA_PANIC: action = "panic"; break;
      case TA_SUPER_PANIC: action = "super_panic"; break;
      default: break;
      }
      sqlite3_bind_text(stmt, 5, action, -1, SQLITE_STATIC);
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
  for (auto& song : km->all_songs->songs) {
    bind_obj_id_or_null(stmt, 1, song);
    sqlite3_bind_text(stmt, 2, song->name.c_str(), -1, SQLITE_STATIC);
    if (song->notes.empty())
      sqlite3_bind_null(stmt, 3);
    else
      sqlite3_bind_text(stmt, 3, song->notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, song->bpm);
    sqlite3_bind_int(stmt, 5, song->clock_on_at_start ? 1 : 0);
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
  for (auto& patch : song->patches) {
    bind_obj_id_or_null(stmt, 1, patch);
    sqlite3_bind_int64(stmt, 2, song->id());
    sqlite3_bind_int(stmt, 3, position++);
    sqlite3_bind_text(stmt, 4, patch->name.c_str(), -1, SQLITE_STATIC);
    bind_obj_id_or_null(stmt, 5, patch->start_message);
    bind_obj_id_or_null(stmt, 6, patch->stop_message);
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
    "    bank_msb, bank_lsb, prog, zone_low, zone_high, xpose, pass_through_sysex)"
    " values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  sqlite3_prepare_v3(db, sql, -1, 0, &stmt, nullptr);
  int position = 0;
  for (auto& conn : patch->connections) {
    bind_obj_id_or_null(stmt, 1, conn);
    sqlite3_bind_int64(stmt, 2, patch->id());
    sqlite3_bind_int(stmt, 3, position++);
    sqlite3_bind_int64(stmt, 4, conn->input->id());
    bind_int_or_null(stmt, 5, conn->input_chan, CONNECTION_ALL_CHANNELS);
    sqlite3_bind_int64(stmt, 6, conn->output->id());
    bind_int_or_null(stmt, 7, conn->output_chan, CONNECTION_ALL_CHANNELS);
    bind_int_or_null(stmt, 8, conn->prog.bank_msb);
    bind_int_or_null(stmt, 9, conn->prog.bank_lsb);
    bind_int_or_null(stmt, 10, conn->prog.prog);
    sqlite3_bind_int(stmt, 11, conn->zone.low);
    sqlite3_bind_int(stmt, 12, conn->zone.high);
    sqlite3_bind_int(stmt, 13, conn->xpose);
    sqlite3_bind_int(stmt, 14, conn->pass_through_sysex ? 1 : 0);
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
    Controller *cc = conn->cc_maps[i];
    if (cc == nullptr)
      continue;

    bind_obj_id_or_null(stmt, 1, cc);
    int j = 2;
    sqlite3_bind_int64(stmt, j++, conn->id());
    sqlite3_bind_int(stmt, j++, cc->cc_num);
    sqlite3_bind_int(stmt, j++, cc->translated_cc_num);
    sqlite3_bind_int(stmt, j++, cc->filtered ? 1 : 0);
    sqlite3_bind_int(stmt, j++, cc->pass_through_0 ? 1 : 0);
    sqlite3_bind_int(stmt, j++, cc->pass_through_127 ? 1 : 0);
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
  for (vector<SetList *>::iterator iter = ++(km->set_lists.begin());
       iter != km->set_lists.end();
       ++iter)
  {
    SetList *set_list = *iter;
    bind_obj_id_or_null(stmt, 1, set_list);
    sqlite3_bind_text(stmt, 2, set_list->name.c_str(), -1, SQLITE_STATIC);
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
  for (auto &song : set_list->songs) {
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
  for (auto &input : km->inputs)
    if (input->id() == id)
      return input;
  set_find_error_message(searcher_name, searcher_id, "input", id);
  return nullptr;
}

Output *Storage::find_output_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &output : km->outputs)
    if (output->id() == id)
      return output;
  set_find_error_message(searcher_name, searcher_id, "output", id);
  return nullptr;
}

Message *Storage::find_message_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &msg : km->messages)
    if (msg->id() == id)
      return msg;
  set_find_error_message(searcher_name, searcher_id, "message", id);
  return nullptr;
}

Song *Storage::find_song_by_id(
  const char * const searcher_name, sqlite3_int64 searcher_id, sqlite3_int64 id
) {
  for (auto &song : km->all_songs->songs)
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
  sprintf(error_buf, "%s (%lld) can't find %s with id %lld\n",
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
  (obj_ptr == nullptr || obj_ptr->id() == UNDEFINED_ID)
    ? sqlite3_bind_null(stmt, col_num)
    : sqlite3_bind_int64(stmt, col_num, obj_ptr->id());
}

void Storage::bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval) {
  val == nullval
    ? sqlite3_bind_null(stmt, col_num)
    : sqlite3_bind_int(stmt, col_num, val);
}

void Storage::extract_id(DBObj *db_obj) {
  if ((db_obj)->id() == UNDEFINED_ID)
    (db_obj)->set_id(sqlite3_last_insert_rowid(db));
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

  byte = hex_to_byte(bytes); bytes += 2;
  msg = byte;

  byte = hex_to_byte(bytes); bytes += 2;
  msg = (msg << 8) + byte;

  byte = hex_to_byte(bytes); bytes += 2;
  msg = (msg << 8) + byte;

  byte = hex_to_byte(bytes);
  msg = (msg << 8) + byte;

  return msg;
}

// FIXME use standard Message format 007f35b0
string Storage::single_message_to_hex_bytes(PmMessage msg) {
  char buf[7];

  sprintf(buf, "00%02x%02x%02x", Pm_MessageData2(msg), Pm_MessageData1(msg), Pm_MessageStatus(msg));
  return string(buf);
}
