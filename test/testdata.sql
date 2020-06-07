insert into instruments (id, type, name, port_name) values
  (1, 0, 'first input', 'port one'),
  (2, 0, 'second input', 'port two'),
  (3, 1, 'first output', 'port one'),
  (4, 1, 'second output', 'port two');

insert into messages (id, name, bytes) values
  (1, 'Tune Request', 'f6'),
  (2, 'Multiple Note-Offs', '804000814000822a7f'),
  (3, '_start', 'b07a00b0077fb1077f'),
  (4, '_stop', 'b2077fb3077fb07a7f');

insert into triggers
  (id, trigger_key_code, input_id, trigger_message_bytes, action, message_id)
values
  (1, 340, null, null, 'panic', null),
  (2, 341, null, null, 'super_panic', null),
  (3, null, 1, 'b0327f', 'next_song', null),
  (4, null, 1, 'b0337f', 'prev_song', null),
  (5, null, 1, 'b0347f', 'next_patch', null),
  (6, null, 1, 'b0357f', 'prev_patch', null),
  (7, null, 1, 'b0367f', null, 1);

insert into songs (id, name, notes) values
  (1, 'To Each His Own', 'example notes'),
  (2, 'Another Song', 'this song has note text
that spans multiple lines'),
  (3, 'Song Without Explicit Patch', null);

insert into patches (id, song_id, position, name, start_message_id, stop_message_id) values
  (1, 1, 0, 'Vanilla Through, Filter Two''s Sustain', null, null),
  (2, 1, 1, 'One Up One Octave and CC Vol -> Pan, Two Down One Octave', null, null),
  (3, 2, 0, 'Two Inputs Merging', null, null),
  (4, 2, 1, 'Split Into Two Outputs', 3, 4);

insert into connections
  (id, patch_id, position, input_id, input_chan, output_id, output_chan,
  bank_msb, bank_lsb, prog, zone_low, zone_high, xpose, pass_through_sysex)
values
  (1, 1, 0, 1, null, 3, null, null, null, null, 0, 127, 0, 0),
  (2, 1, 1, 2, null, 4, null, 3, 2, 12, 0, 127, 0, 0),
  (3, 2, 0, 1, null, 3, null, null, null, null, 0, 127, 12, 0),
  (4, 2, 1, 2, null, 4, null, null, 5, null, 0, 127, -12, 0),
  (5, 3, 0, 1, 2, 3, 3, null, null, null, 0, 63, 12, 0),
  (6, 3, 1, 2, 0, 4, 4, null, null, null, 64, 127, -12, 0),
  (7, 4, 0, 1, 2, 3, 3, null, null, null, 0, 63, 12, 0),
  (8, 4, 1, 2, 0, 4, 4, null, null, null, 64, 127, -12, 0);

insert into controller_mappings
  (id, connection_id, cc_num, translated_cc_num, filtered,
   pass_through_0, pass_through_127, min_in, max_in, min_out, max_out)
values
  (1, 2, 64, 64, 1, 1, 1, 0, 127, 0, 127),
  (2, 3, 7, 10, 0, 0, 0, 1, 120, 40, 50);

insert into set_lists (id, name) values
  (1, 'Set List One'),
  (2, 'Set List Two');

insert into set_lists_songs (set_list_id, song_id, position) values
  (1, 1, 0),
  (1, 2, 1),
  (2, 2, 0),
  (2, 1, 1);
