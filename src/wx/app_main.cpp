#include "app.h"

/*
 * This macro call, which generates a `main` function, call can't live in
 * app.cpp. That's because the Catch2 macro system used by the tests
 * generates its own `main` function, clashing with this one. We keep this
 * `main` in this file so we can exclude it when compiling and running the
 * test code. See the Makefile.
 */
wxIMPLEMENT_APP(App);
