#pragma once
#include <stdbool.h>

bool path_has_extension(const char* path, const char* extension);
void path_get_filename(const char* path, char* output);

