#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include <sqlite3.h>
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

private:
  sqlite3 *db;
  KeyMaster *km;
  string error_str;
  int max_patch_id;
  int max_conn_id;

  void load_instruments();
  void load_messages();
  void load_triggers();
  void load_songs();
  void load_patches(Song *);
  void load_connections(Patch *);
  void load_controller_mappings(Connection *);
  void load_set_lists();
  void load_set_list_songs(SetList *);

  void save_instruments();
  void save_messages();
  void save_triggers();
  void save_trigger(sqlite3_stmt *, Input *, Trigger *);
  void save_songs();
  void save_patches(Song *);
  void save_connections(Patch *);
  void save_controller_mappings(Connection *);
  void save_set_lists();
  void save_set_list_songs(SetList *);

  void create_default_patches();
  void create_default_patch(Song *);

  PmDeviceID find_device(const char *name, int device_type);
  Input *find_input_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Output *find_output_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Message *find_message_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  Song *find_song_by_id(const char * const, sqlite3_int64, sqlite3_int64);
  void set_find_error_message(const char * const, sqlite3_int64,
                              const char * const, sqlite3_int64);

  int compare_device_names(const char *name1, const char *name2);

  // SQL statement helpers
  int int_or_null(sqlite3_stmt *stmt, int col_num, int null_val=UNDEFINED);
  sqlite3_int64 id_or_null(sqlite3_stmt *stmt, int col_num, sqlite3_int64 null_val=UNDEFINED_ID);
  const char *text_or_null(sqlite3_stmt *stmt, int col_num, const char *null_val);
  void bind_obj_id_or_null(sqlite3_stmt *stmt, int col_num, DBObj *obj_ptr);
  void bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval=UNDEFINED);
  void extract_id(DBObj *db_obj);

  PmMessage km_message_from_bytes(char *);
  string message_to_byte_str(Message *);
  string km_message_to_bytes(PmMessage msg);
};

#endif /* STORAGE_H */
