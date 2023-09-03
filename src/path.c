#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include <shlobj.h>

#include "path.h"
#include "logging.h"

bool path_has_extension(const char* path, const char* extension) {
    uint32_t pos = strlen(path);
    uint16_t ext_length = strlen(extension);

    // File extension is longer than input string.
    if (ext_length > pos) {
        return false;
    }
    return (strncmp(&path[pos - ext_length], extension, ext_length) == 0);
}

void path_get_filename(const char* path, char* output) {
    int32_t pos = strlen(path);
    // We need to check if it's over -1 so that if there are no backslashes,
    // pos = -1 and it just copies the whole string.
    while(path[pos] != '\\' && path[pos] != '/' && pos > -1) {
        pos--;
    }
    strcpy(output, &path[pos] + 1);
}

