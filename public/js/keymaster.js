const $ = jQuery;

const CONN_HEADERS = "<tr>\n  <th>Input</th>\n  <th>Chan</th>\n  <th>Output</th>\n  <th>Chan</th>\n  <th>Prog</th>\n  <th>Zone</th>\n  <th>Xpose</th>\n  <th>Filter</th>\n</tr>";

const COLOR_SCHEMES = ['default', 'green', 'amber', 'blue'];

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

function connection_row(conn) {
  var vals = ['input', 'input_chan', 'output', 'output_chan', 'pc', 'zone', 'xpose', 'filter']
      .map((key, idx) => {
        if (key == 'zone') {
          z = conn.zone;
          if (z.low == 0 && z.high == 127)
            return '';
          else
            return '' + z.low + ' - ' + z.high;
        }
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

function keypress(action) {
  return $.getJSON(action, function(data) {
    list('song-lists', data['lists'], data['list']);
    list('songs', data['songs'], maybe_name(data, 'song'));
    list('triggers', data['triggers']);
    if (data['song'] != null) {
      notes_or_help(data['song']['notes']);
      list('song', data['song']['patches'], maybe_name(data, 'patch'));
      if (data['patch'] != null) {
        connection_rows(data['patch']['connections']);
      }
    }
    if (data['message'] != null) {
      return message(data['message']);
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
  console.log("TODO: goto_song");
}

function goto_set_list() {
  console.log("TODO: goto_set_list");
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
  'g': goto_song,
  't': goto_set_list
}

function bind_keypress(key, val) {
  return $(document).bind('keydown', key, function() {
    return keypress(val);
  });
};

for (key in KEYMASTER_KEY_BINDINGS) {
  val = KEYMASTER_KEY_BINDINGS[key];
  bind_keypress(key, val);
}

function bind_local_call(key, func) {
  $(document).bind('keydown', key, function() {
    return func();
  });
}

for (key in LOCAL_KEY_BINDINGS) {
  func = LOCAL_KEY_BINDINGS[key];
  bind_local_call(key, func);

// ================ initialize data ================

keypress('status');
