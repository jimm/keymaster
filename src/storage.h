#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include <portmidi.h>
#include "keymaster.h"

using namespace std;

class Storage {
public:
  Storage(const char *path);
  ~Storage();

  void initialize();            // public for testing

  KeyMaster *load(bool testing = false);
  void save(KeyMaster *km, bool testing = false);
  bool has_error();
  string error();

  int schema_version();         // for testing

private:
  sqlite3 *db;
  KeyMaster *km;
  int loading_version;
  string error_str;

  void load_schema_version();
  void load_instruments();
  void load_velocity_curves();
  void load_messages();
  void load_triggers();
  void load_songs();
  void load_patches(Song *);
  void load_connections(Patch *);
  void load_controller_mappings(Connection *);
  void load_set_lists();
  void load_set_list_songs(SetList *);

  void save_schema_version();
  void save_instruments();
  void save_velocity_curves();
  void save_velocity_curves(vector<Curve *> &curves);
  void save_messages();
  void save_triggers();
  void save_songs();
  void save_patches(Song *);
  void save_connections(Patch *);
  void save_controller_mappings(Connection *);
  void save_set_lists();
  void save_set_list_songs(SetList *);

  void create_default_patches();
  void create_default_patch(Song *);

  Input *find_input_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Output *find_output_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Message *find_message_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Song *find_song_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  void set_find_error_message(const char * const, sqlite3_int64,
                              const char * const, sqlite3_int64);

  // SQL statement helpers
  int int_or_null(sqlite3_stmt *stmt, int col_num, int null_val=UNDEFINED);
  sqlite3_int64 id_or_null(sqlite3_stmt *stmt, int col_num, sqlite3_int64 null_val=UNDEFINED_ID);
  const char *text_or_null(sqlite3_stmt *stmt, int col_num, const char *null_val);
  void bind_obj_id_or_null(sqlite3_stmt *stmt, int col_num, DBObj *obj_ptr);
  void bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval=UNDEFINED);
  void extract_id(DBObj *db_obj);

  PmMessage single_message_from_hex_bytes(char *);
  string single_message_to_hex_bytes(PmMessage msg);
};

const char * last_loaded_file_path();

#endif /* STORAGE_H */
