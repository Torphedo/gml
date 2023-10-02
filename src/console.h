#pragma once
#include <stdbool.h>

// Utilities for enabling color and spawning a working console in a process
// without one.

// Create a new console, redirect streams to it, enable ANSI control codes,
// set the output mode to UTF-8 w/ Unicode, and show the window.
bool console_setup();

// Enables ANSI escape codes, mostly useful for enabling color output.
bool enable_ansi_codes();

