const $ = jQuery;

const CONN_HEADERS = "<tr>\n  <th>Input</th>\n  <th>Chan</th>\n  <th>Output</th>\n  <th>Chan</th>\n  <th>Prog</th>\n  <th>Zone</th>\n  <th>Xpose</th>\n  <th>Filter</th>\n</tr>";

const COLOR_SCHEMES = ['default', 'green', 'amber', 'blue'];

const UNDEFINED = -1;

var keymaster;

var color_scheme_index = 0;

function notes_or_help(notes) {
  if (notes !== undefined && notes != "") {
    $('#notes').html(notes);
    return;
  }

  $('#notes_or_help').html(`j, down, space  - next patch
k, up           - prev patch
n, left         - next song
p, right        - prev song

g   - goto song
t   - goto song list

c   - cycle screen colors

h   - help
ESC - panic

q   - quit
`);
}

function ensureElementInViewport(el) {
  parent = el.parent()[0];
  el = el[0];
  if (el === undefined)
    return;

  var rect = el.getBoundingClientRect();
  var parent_rect = parent.getBoundingClientRect();
  if (rect.top < parent_rect.top)
    el.scrollIntoView(true);
  else if (rect.bottom > parent_rect.bottom)
    el.scrollIntoView(false);
}

function list_item(val, highlighted_value) {
  var classes;
  classes = val === highlighted_value ? "selected reverse-" + COLOR_SCHEMES[color_scheme_index] : '';
  return "<li class=\"" + classes + "\">" + val + "</li>";
};

function list(id, vals, highlighted_value) {
  var list = vals.map((val, idx) => list_item(val, highlighted_value));
  $(`ul#${id}`).html(list.join("\n"));
  ensureElementInViewport($(`ul#${id} li.selected`));
};

function zone_to_str(zone) {
  if (zone.low == 0 && zone.high == 127)
    return '';
  else
    return '' + zone.low + ' - ' + zone.high;
}

function program_to_str(pc) {
  return "" + pc;
}

function connection_row(conn) {
  var vals = ['input', 'input_chan', 'output', 'output_chan', 'pc', 'zone', 'xpose', 'filter']
      .map((key, idx) => {
        if (key == 'zone')
          return zone_to_str(conn.zone);
        else if (key == 'pc')
          return program_to_str(conn.pc);
        else
          return conn[key];
      });
  return "<tr><td>" + (vals.join('</td><td>')) + "</td></tr>";
};

function connection_rows(connections) {
  var conn, rows;
  rows = (function() {
    var _i, _len, _results;
    _results = [];
    for (_i = 0, _len = connections.length; _i < _len; _i++) {
      conn = connections[_i];
      _results.push(connection_row(conn));
    }
    return _results;
  })();
  $('#patch').html(CONN_HEADERS + "\n" + rows.join("\n"));
  return set_colors();
};

function maybe_name(data, key) {
  return data[key] ? data[key]['name'] : '';
};

function message(str) {
  return $('#message').html(str);
};

function perform_action(action) {
  return $.getJSON(action, function(data) {
    keymaster = data;
    list('song-lists', keymaster['lists'], keymaster['list']);
    list('songs', keymaster['songs'], maybe_name(keymaster, 'song'));
    list('triggers', keymaster['triggers']);
    if (keymaster['song'] != null) {
      notes_or_help(keymaster['song']['notes']);
      list('song', keymaster['song']['patches'], maybe_name(keymaster, 'patch'));
      if (keymaster['patch'] != null) {
        connection_rows(keymaster['patch']['connections']);
      }
    }
    if (keymaster['message'] != null) {
      return message(keymaster['message']);
    }
  });
};

function remove_colors() {
  if (color_scheme_index >= 0) {
    var base_class;
    base_class = COLOR_SCHEMES[color_scheme_index];
    $('body').removeClass(base_class);
    $('.selected, th, td#appname').removeClass("reverse-" + base_class);
    $('tr, td, th').removeClass("" + base_class + "-border");
  }
};

function set_colors() {
  var base_class;
  base_class = COLOR_SCHEMES[color_scheme_index];
  $('body').addClass(base_class);
  $('.selected, th, td#appname').addClass("reverse-" + base_class);
  return $('tr, td, th').addClass("" + base_class + "-border");
};

function cycle_colors() {
  remove_colors();
  color_scheme_index = (color_scheme_index + 1) % COLOR_SCHEMES.length;
  return set_colors();
};

// ================ local funcs ================

function goto_song() {
  alert("TODO: goto_song");
}

function goto_set_list() {
  alert("TODO: goto_set_list");
}

function edit_song() {
  const editor = $('#editsong');
  editor.show();
  $('#songname').focus();
  // $('#editsong button').click(function() { editor.hide(); });
}

function edit_patch() {
  $('#editpatch').show();
  $('#patchname').focus();
}

function help() {
  // TODO
}

// ================ bind keys ================

const KEYMASTER_KEY_BINDINGS = {
  'j': 'next_patch',
  'down': 'next_patch',
  'k': 'prev_patch',
  'up': 'prev_patch',
  'n': 'next_song',
  'left': 'next_song',
  'p': 'prev_song',
  'right': 'prev_song',
  'esc': 'panic'
};

const LOCAL_KEY_BINDINGS = {
  'c': cycle_colors,
  'g': goto_song,
  't': goto_set_list,
  's': edit_song,
  'a': edit_patch,
  '?': help
}

function bind_keypress(key, val) {
  return $(document).bind('keydown', key, function() {
    return perform_action(val);
  });
};

for (key in KEYMASTER_KEY_BINDINGS) {
  val = KEYMASTER_KEY_BINDINGS[key];
  bind_keypress(key, val);
}

function bind_local_call(key, func) {
  $(document).bind('keydown', key, function(event) {
    event.preventDefault();
    return func();
  });
}

for (key in LOCAL_KEY_BINDINGS) {
  func = LOCAL_KEY_BINDINGS[key];
  bind_local_call(key, func);
}

// ================ initialize keymaster ================

perform_action('status');
