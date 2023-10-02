#include <string.h>

// Reduce the size of Windows.h to improve compile time
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMB
#include <windows.h>

#include "path.h"

// Generic path utilities to manipulate and do checks on inputted C strings.
// See path.h

bool path_has_extension(const char* path, const char* extension) {
    uint32_t pos = strnlen(path, MAX_PATH);
    uint16_t ext_length = strnlen(extension, MAX_PATH);

    // File extension is longer than input string.
    if (ext_length > pos) {
        return false;
    }
    return (strncmp(&path[pos - ext_length], extension, ext_length) == 0);
}

void path_get_filename(const char* path, char* output) {
    int64_t pos = strnlen(path, MAX_PATH);
    // We need to check if it's over -1 so that if there are no backslashes,
    // pos = -1 and it just copies the whole string.
    while(path[pos] != '\\' && path[pos] != '/' && pos > -1) {
        pos--;
    }
    strncpy(output, &path[pos] + 1, MAX_PATH);
}

void path_truncate(char* path, uint16_t pos) {
    path[--pos] = 0; // Removes last character to take care of trailing "\\" or "/".
    while(path[pos] != '\\' && path[pos] != '/') {
        path[pos--] = 0;
    }
}

