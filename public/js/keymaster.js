const $ = jQuery;

const CONN_HEADERS = "<tr>\n  <th>Input</th>\n  <th>Chan</th>\n  <th>Output</th>\n  <th>Chan</th>\n  <th>Prog</th>\n  <th>Zone</th>\n  <th>Xpose</th>\n  <th>Filter</th>\n</tr>";

const COLOR_SCHEMES = ['default', 'green', 'amber', 'blue'];

const KEY_BINDINGS = {
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

function list_item(val, highlighted_value) {
  var classes;
  classes = val === highlighted_value ? "selected reverse-" + COLOR_SCHEMES[color_scheme_index] : '';
  return "<li class=\"" + classes + "\">" + val + "</li>";
};

function list(id, vals, highlighted_value) {
  var lis, val;
  lis = (function() {
    var _i, _len, _results;
    _results = [];
    for (_i = 0, _len = vals.length; _i < _len; _i++) {
      val = vals[_i];
      _results.push(list_item(val, highlighted_value));
    }
    return _results;
  })();
  return $('#' + id).html(lis.join("\n"));
};

function connection_row(conn) {
  var key, vals, z;
  vals = (function() {
    var _i, _len, _ref, _results;
    _ref = ['input', 'input_chan', 'output', 'output_chan', 'pc', 'zone', 'xpose', 'filter'];
    _results = [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      key = _ref[_i];
      if (key == 'zone') {
        z = conn['zone'];
        if (z.low == 0 && z.high == 127)
          _results.push('');
        else
          _results.push('' + z.low + ' - ' + z.high);
      }
      else
        _results.push(conn[key]);
    }
    return _results;
  })();
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
  if (data[key]) {
    return data[key]['name'];
  } else {
    return '';
  }
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

function bind_keypress(key, val) {
  return $(document).bind('keydown', key, function() {
    return keypress(val);
  });
};

for (key in KEY_BINDINGS) {
  val = KEY_BINDINGS[key];
  bind_keypress(key, val);
}

$(document).bind('keydown', 'c', function() {
  return cycle_colors();
});

keypress('status');
