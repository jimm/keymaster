[The repo formerly named "keymaster" that used to be here has been renamed
to "keymaster\_objc". It has moved
[here](https://github.com/jimm/keymaster_objc). This project used to be
named "SeaMaster".]

# KeyMaster

KeyMaster is a MIDI processing and patching system. It allows a musician to
reconfigure a MIDI setup instantaneously and modify the MIDI data in real
time.

With KeyMaster a performer can route MIDI between keyboards, split
controlling keyboards, layer MIDI channels, transpose them, send program
changes and System Exclusive messages, limit controller and velocity values,
modify controllers, and much more. At the stomp of a foot switch (or any
other MIDI event), an entire MIDI system can be totally reconfigured.

For more information see the [wiki](https://github.com/jimm/keymaster/wiki).

![Screenshot](https://raw.githubusercontent.com/wiki/jimm/keymaster/images/km_screen_shot.png)

# Key Bindings

## Movement

- **j**, **down**, **space** - Next patch
- **k**, **up** - Previous patch
- **n**, **right** - Next song
- **p**, **left** - Previous song
- **g** - Goto song
- **l** - Goto song list

## Performance

- **ESC** - Panic
- **ESC ESC** - Send individual note off messages on all channels

## Editing

- **e p** - Edit patch
- **e s** - Edit song
- **e l** - Edit song list ("All songs" list is not editable)
- **e m** - Edit message
- **e t** - Edit trigger
- **n p** - New patch
- **n s** - New song
- **n l** - New song list
- **n m** - New message
- **n t** - New trigger
- **l** - Load a file
- **s** - Save to a file
- **r** - Reload previously loaded or last saved file

## MIDI Monitor

- **m** - Open MIDI monitor window
- **ESC** - Close MIDI monitor window
- **c** - Toggle display of clock messages (default is off)

## Miscellaneous

- **m** - Open MIDI monitor
- **v** - Toggle view

- **h**, **?** - Help; any key closes the help window
- **q** - Quit

# To Do / Bugs

See the [To Do](https://github.com/jimm/keymaster/wiki/To-Do) list on the
Wiki.
