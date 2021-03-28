drop table if exists schema_version;
drop table if exists set_lists;
drop table if exists set_lists_songs;
drop table if exists controller_mappings;
drop table if exists connections;
drop table if exists patches;
drop table if exists songs;
drop table if exists triggers;
drop table if exists messages;
drop table if exists velocity_curves;
drop table if exists instruments;

create table instruments (
  id integer primary key,
  type integer not null default 0,  -- 0 == input, 1 == output
  name text,
  device_name text
);

create table velocity_curves (
  id integer primary key,
  name text,
  short_name text,
  curve text                    -- 128 bytes encoded as two-digit hex chars
);

create table messages (
  id integer primary key,
  name text not null,
  bytes text not null
);

create table triggers (
  id integer primary key,
  trigger_key_code integer,     -- either this or bytes must not be NULL
                                -- both may be not NULL
  input_id integer references instruments(id),
  trigger_message_bytes text,
  action text,
  message_id integer references messages(id)
);

create table songs (
  id integer primary key,
  name text,
  notes text,
  bpm integer not null default 120,
  clock_on_at_start integer not null default 0 -- boolean
);

create table patches (
  id integer primary key,
  song_id integer not null references songs(id),
  position integer not null default 0,
  name text,
  start_message_id integer references messages(id),
  stop_message_id integer references messages(id)
);

create table connections (
  id integer primary key,
  patch_id integer not null references patches(id),
  position integer not null default 0,
  input_id integer not null,
  input_chan integer,
  output_id integer not null,
  output_chan integer,
  bank_msb int,
  bank_lsb int,
  prog int,
  zone_low integer not null default 0,
  zone_high integer not null default 127,
  xpose integer not null default 0,
  velocity_curve_id integer not null references velocity_curves(id),
  -- pass-through booleans
  note integer not null default 1,
  poly_pressure integer not null default 1,
  chan_pressure integer not null default 1,
  program_change integer not null default 0,
  pitch_bend integer not null default 1,
  controller integer not null default 1,
  song_pointer integer not null default 1,
  song_select integer not null default 1,
  tune_request integer not null default 1,
  sysex integer not null default 0,
  clock integer not null default 1,
  start_continue_stop integer not null default 1,
  system_reset integer not null default 1
);

create table controller_mappings (
  id integer primary key,
  connection_id integer not null references connections(id),
  cc_num integer not null,
  translated_cc_num integer not null,
  filtered integer not null default 0, -- boolean
  pass_through_0 integer not null default 1, -- boolean
  pass_through_127 integer not null default 1, -- boolean
  min_in integer not null,
  max_in integer not null,
  min_out integer not null,
  max_out integer not null
);

create table set_lists (
  id integer primary key,
  name text
);

create table set_lists_songs (
  set_list_id integer not null references set_lists(id),
  song_id integer not null null references songs(id),
  position integer not null default 0
);

create table schema_version (
  version integer not null
);
insert into schema_version (version) values (0);
