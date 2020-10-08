insert into instruments (id, type, name, device_name) values
  (1, 0, 'first input', 'port one'),
  (2, 0, 'second input', 'port two'),
  (3, 1, 'first output', 'port one'),
  (4, 1, 'second output', 'port two');

insert into messages (id, name, bytes) values
  (1, 'Tune Request', '000000f6'),
  (2, 'Multiple Note-Offs', '0000408000004081007f2a82'),
  (3, '_start', '00007ab0007f07b0007f07b1'),
  (4, '_stop', '007f07b2007f07b3007fa7b0');

insert into triggers
  (id, trigger_key_code, input_id, trigger_message_bytes, action, message_id)
values
  (1, 340, null, null, 'panic', null),
  (2, 341, null, null, 'super_panic', null),
  (3, null, 1, '007f32b0', 'next_song', null),
  (4, null, 1, '007f33b0', 'prev_song', null),
  (5, null, 1, '007f34b0', 'next_patch', null),
  (6, null, 1, '007f35b0', 'prev_patch', null),
  (7, null, 1, '007f36b0', null, 1);

insert into songs (id, name, notes, bpm, clock_on_at_start) values
  (1, 'To Each His Own', 'example notes', 82, 1),
  (2, 'Another Song', 'this song has note text
that spans multiple lines', 120, 0),
  (3, 'Song Without Explicit Patch', null, 120, 0);

insert into patches (id, song_id, position, name, start_message_id, stop_message_id) values
  (1, 1, 0, 'Vanilla Through, Filter Two''s Sustain', null, null),
  (2, 1, 1, 'One Up One Octave and CC Vol -> Pan, Two Down One Octave', null, null),
  (3, 2, 0, 'Two Inputs Merging', null, null),
  (4, 2, 1, 'Split Into Two Outputs', 3, 4);

insert into connections
  (id, patch_id, position, input_id, input_chan, output_id, output_chan,
  bank_msb, bank_lsb, prog, zone_low, zone_high, xpose, velocity_xpose,
  note, poly_pressure, chan_pressure, program_change, pitch_bend,
  controller, song_pointer, song_select, tune_request, sysex,
  clock, start_continue_stop, system_reset)
values
  (1, 1, 0, 1, null, 3, null, null, null, null, 0, 127, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (2, 1, 1, 2, null, 4, null, 3, 2, 12, 0, 127, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (3, 2, 0, 1, null, 3, null, null, null, null, 0, 127, 12, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (4, 2, 1, 2, null, 4, null, null, 5, null, 0, 127, -12, 42, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (5, 3, 0, 1, 2, 3, 3, null, null, null, 0, 63, 12, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (6, 3, 1, 2, 0, 4, 4, null, null, null, 64, 127, -12, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (7, 4, 0, 1, 2, 3, 3, null, null, null, 0, 63, 12, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1),
  (8, 4, 1, 2, 0, 4, 4, null, null, null, 64, 127, -12, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1);

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
