#ifndef SET_LIST_H
#define SET_LIST_H

#include <vector>
#include "db_obj.h"
#include "song.h"
#include "observer.h"

using namespace std;

class SetList : public DBObj, public Named, public Observer {
public:
  SetList(sqlite3_int64 id, const char *name);
  ~SetList();

  inline vector<Song *> &songs() { return _songs; }

  void add_song(Song *song);
  void remove_song(Song *song);

  void set_songs(vector<Song *>&other_set_list);

  void update(Observable *o, void *arg);

private:
  vector<Song *> _songs;
};

#endif /* SET_LIST_H */
