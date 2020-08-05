#ifndef MACROS_H
#define MACROS_H

// TITLE_STR is necessary because for some reason if a string does not end
// with a space, it gets truncated by one character. Even stranger, if the
// string contains a space it gets chopped off at the first space if there
// isn't another space at the end.
#define TITLE_STR(str) (str" ")

#define POS(row, col) wxGBPosition(row, col)
#define SPAN(rowspan, colspan) wxGBSpan(rowspan, colspan)

#define TEXT_EDIT_HEIGHT 22
#define NAME_WIDTH 250
#define NAME_HEIGHT TEXT_EDIT_HEIGHT

#define NAME_CTRL_SIZE (wxSize(NAME_WIDTH, NAME_HEIGHT))

#endif /* MACROS_H */
