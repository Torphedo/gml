#pragma once
#include <stdbool.h>
#include <stdint.h>

// Generic path utilities to manipulate and do checks on inputted C strings.

bool path_has_extension(const char* path, const char* extension);
void path_get_filename(const char* path, char* output);
void path_truncate(char* path, uint16_t pos);

