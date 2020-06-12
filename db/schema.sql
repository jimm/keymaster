drop table if exists set_lists;
drop table if exists set_lists_songs;
drop table if exists controller_mappings;
drop table if exists connections;
drop table if exists patches;
drop table if exists songs;
drop table if exists triggers;
drop table if exists messages;
drop table if exists instruments;

create table instruments (
  id integer primary key,
  type integer not null default 0,  -- 0 == input, 1 == output
  name text,
  device_name text
);

create table messages (
  id integer primary key,
  name name not null,
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
  notes text
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
  input_chan,
  output_id integer not null,
  output_chan,
  bank_msb int,
  bank_lsb int,
  prog int,
  zone_low integer not null,
  zone_high integer not null,
  xpose integer not null,
  pass_through_sysex integer not null default 0 -- boolean
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
