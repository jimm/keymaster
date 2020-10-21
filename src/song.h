#ifndef SONG_H
#define SONG_H

#include <vector>
#include "db_obj.h"
#include "patch.h"

using namespace std;

class Song : public DBObj, public Named, public Observer {
public:
  Song(sqlite3_int64 id, const char *name);
  ~Song();

  inline vector<Patch *> &patches() { return _patches; }
  inline string &notes() { return _notes; }
  inline float bpm() { return _bpm; }
  inline bool clock_on_at_start() { return _clock_on_at_start; }

  void set_notes(string &notes);
  void set_notes(const char *notes);
  void set_bpm(float bpm);
  void set_clock_on_at_start(bool val);

  void add_patch(Patch *patch);
  void remove_patch(Patch *patch);

  void update(Observable *o, void *arg);

private:
  vector<Patch *> _patches;
  string _notes;
  float _bpm;
  bool _clock_on_at_start;
};

#endif /* SONG_H */
