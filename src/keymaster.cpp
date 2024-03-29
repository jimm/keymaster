#include <stdlib.h>
#include "keymaster.h"
#include "cursor.h"
#include "device.h"
#include "vector_utils.h"

#define PATCH_STOP  {if (_cursor->patch() != nullptr) _cursor->patch()->stop();}
#define PATCH_START {update_clock(); if (_cursor->patch() != nullptr) _cursor->patch()->start();}


static KeyMaster *km_instance = nullptr;

KeyMaster *KeyMaster_instance() {
  return km_instance;
}

// ================ allocation ================

KeyMaster::KeyMaster()
  : _running(false), _testing(false), _modified(false), _clock(_inputs)
{
  _set_lists.push_back(new SetList(UNDEFINED_ID, (char *)"All Songs"));
  _cursor = new Cursor(this);
  km_instance = this;
}

KeyMaster::~KeyMaster() {
  if (km_instance == this)
    km_instance = nullptr;

  for (auto& in : _inputs)
    delete in;
  for (auto& out : _outputs)
    delete out;
  for (auto& t : _triggers)
    delete t;
  for (auto& song : all_songs()->songs())
    delete song;
  for (auto& set_list : _set_lists)
    delete set_list;
  for (auto& msg : _messages)
    delete msg;
  for (auto& curve : _velocity_curves)
    delete curve;
}

// ================ accessors ================

void KeyMaster::add_input(Input *input) {
  _inputs.push_back(input);
  changed();
}

void KeyMaster::add_output(Output *output) {
  _outputs.push_back(output);
  changed();
}

void KeyMaster::add_message(Message *message) {
  message->add_observer(this);
  _messages.push_back(message);
  changed();
}

void KeyMaster::remove_message(Message *message) {
  message->remove_observer(this);
  erase(_messages, message);
  delete message;
  changed();
}

void KeyMaster::add_velocity_curve(Curve *velocity_curve) {
  _velocity_curves.push_back(velocity_curve);
  changed();
}

void KeyMaster::remove_velocity_curve(Curve *velocity_curve) {
  erase(_velocity_curves, velocity_curve);
  delete velocity_curve;
  changed();
}

Curve * KeyMaster::velocity_curve_with_name(const char *name) {
  for (auto &curve : _velocity_curves)
    if (curve->name() == name)
      return curve;
  return nullptr;
}

Curve * KeyMaster::velocity_curve_with_id(sqlite3_int64 id) {
  for (auto &curve : _velocity_curves)
    if (curve->id() == id)
      return curve;
  return nullptr;
}

void KeyMaster::add_trigger(Trigger *trigger) {
  trigger->add_observer(this);
  _triggers.push_back(trigger);
  changed();
}

void KeyMaster::remove_trigger(Trigger *trigger) {
  trigger->remove_observer(this);
  erase(_triggers, trigger);
  delete trigger;
  changed();
}

void KeyMaster::add_set_list(SetList *set_list) {
  set_list->add_observer(this);
  _set_lists.push_back(set_list);
  changed();
}

void KeyMaster::remove_set_list(SetList *set_list) {
  set_list->remove_observer(this);
  erase(_set_lists, set_list);
  delete set_list;
  changed();
}

// ================ observer / observable ================

// Only called by storage after data is loaded or saved.
void KeyMaster::clear_modified() {
  _modified = false;
  Observable::changed((void *)&_modified);
}

void KeyMaster::update(Observable *_o, void *_arg) {
  changed();
}

void KeyMaster::changed(void *_arg) {
  _modified = true;
  Observable::changed((void *)&_modified);
}

// ================ running ================

void KeyMaster::start() {
  _cursor->init();
  for (auto& out : _outputs)
    out->start();
  for (auto& in : _inputs)
    in->start();
  _running = true;
  PATCH_START;
}

void KeyMaster::stop() {
  stop_clock();
  PATCH_STOP;
  _running = false;
  for (auto& in : _inputs)
    in->stop();
  for (auto& out : _outputs)
    out->stop();
}

// ================ clock ================

void KeyMaster::update_clock() {
  Song *curr_song = _cursor->song();
  if (curr_song == nullptr || !curr_song->clock_on_at_start())
    stop_clock();
  else {
    set_clock_bpm(curr_song->bpm());
    start_clock();
  }
}

// ================ initialization ================

void KeyMaster::initialize() {
  load_instruments();
  generate_default_curves(this->_velocity_curves);
  create_songs();
  _modified = false;
}

void KeyMaster::load_instruments() {
  if (_testing)
    return;

  for (auto &iter : devices()) {
    const PmDeviceInfo *info = iter.second;
    if (info->input)
      _inputs.push_back(new Input(UNDEFINED_ID, iter.first, info->name));
    if (info->output)
      _outputs.push_back(new Output(UNDEFINED_ID, iter.first, info->name));
  }
}

void KeyMaster::create_songs() {
  char name[BUFSIZ];

  for (auto& input : _inputs) {
    // this input to each individual output
    int output_num = 1;
    for (auto& output : _outputs) {
      snprintf(name, BUFSIZ, "%s -> %s", input->name().c_str(), output->name().c_str());
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      Connection *conn = new Connection(UNDEFINED_ID, input, CONNECTION_ALL_CHANNELS,
                                        output, CONNECTION_ALL_CHANNELS);
      patch->add_connection(conn);

      ++output_num;
    }

    if (_outputs.size() > 1) {
      // one more song: this input to all _outputs at once
      snprintf(name, BUFSIZ, "%s -> all _outputs", input->name().c_str());
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      for (auto& output : _outputs) {
        Connection *conn = new Connection(
          UNDEFINED,
          input, CONNECTION_ALL_CHANNELS,
          output, CONNECTION_ALL_CHANNELS);
        patch->add_connection(conn);
      }
    }
  }
}

// ================ movement ================

void KeyMaster::next_patch() {
  PATCH_STOP;
  _cursor->next_patch();
  PATCH_START;
}

void KeyMaster::prev_patch() {
  PATCH_STOP;
  _cursor->prev_patch();
  PATCH_START;
}

void KeyMaster::next_song() {
  PATCH_STOP;
  _cursor->next_song();
  PATCH_START;
}

void KeyMaster::prev_song() {
  PATCH_STOP;
  _cursor->prev_song();
  PATCH_START;
}

// ================ going places ================

void KeyMaster::goto_song(Song *song) {
  PATCH_STOP;
  _cursor->goto_song(song);
  PATCH_START;
}

void KeyMaster::goto_patch(Patch *patch) {
  PATCH_STOP;
  _cursor->goto_patch(patch);
  PATCH_START;
}

void KeyMaster::goto_song(string name_regex) {
  PATCH_STOP;
  _cursor->goto_song(name_regex);
  PATCH_START;
}

void KeyMaster::goto_set_list(string name_regex) {
  PATCH_STOP;
  _cursor->goto_set_list(name_regex);
  PATCH_START;
}

void KeyMaster::jump_to_set_list_index(int i) {
  if (i == _cursor->set_list_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_set_list_index(i);
  PATCH_START;
}

void KeyMaster::jump_to_song_index(int i) {
  if (i == _cursor->song_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_song_index(i);
  PATCH_START;
}

void KeyMaster::jump_to_patch_index(int i) {
  if (i == _cursor->patch_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_patch_index(i);
  PATCH_START;
}

// ================ doing things ================

void KeyMaster::panic(bool send_notes_off) {
  PmEvent buf[128];

  memset(buf, 0, 128 * sizeof(PmEvent));
  if (send_notes_off) {
    for (int i = 0; i < 128; ++i)
      buf[i].timestamp = 0;
    for (int i = 0; i < 16; ++i) {
      for (int j = 0; j < 128; ++j)
        buf[j].message = Pm_Message(NOTE_OFF + i, j, 0);
      for (auto& out : _outputs)
        out->write(buf, 128);
    }
  }
  else {
    for (int i = 0; i < 16; ++i) {
      buf[i].message = Pm_Message(CONTROLLER + i, CM_ALL_NOTES_OFF, 0);
      buf[i].timestamp = 0;
    }
    for (auto& out : _outputs)
      out->write(buf, 16);
  }
}

// ================ helpers ================

bool songNameComparator(Song *s1, Song *s2) {
  return s1->name() < s2->name();
}

void KeyMaster::sort_all_songs() {
  sort(all_songs()->songs().begin(), all_songs()->songs().end(), songNameComparator);
}
