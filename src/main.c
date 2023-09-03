#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Reduce the size of Windows.h to improve compile time
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMB
#include <windows.h>
#include <direct.h>

#include "logging.h"
#include "self_inject.h"
#include "path.h"
#include "console.h"
#include "hooks.h"

typedef int (WINAPI *exe_entry_proc)(void);

DWORD __stdcall bootstrap(void* module_handle) {
    if (module_handle == NULL) {
        // Something is SERIOUSLY wrong and we shouldn't have even been called.
        // This will never happen.
        return EXIT_FAILURE;
    }

    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)module_handle;
    IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)((uintptr_t)module_handle + dos_header->e_lfanew);
    exe_entry_proc entry_point = (exe_entry_proc)((uintptr_t)module_handle + nt_header->OptionalHeader.AddressOfEntryPoint);

    console_setup(32000);
    LOG_MSG(info, "Console enabled\n");

    hook_io();

    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    if (GetModuleHandleA("gml.exe") == NULL) {
        LOG_MSG(info, "entry point called.\n");
        return 0;
    }

    char* target_name = argv[1];
    char* shell_cmd = argv[2];
    enable_ansi_codes();

    if (argc == 1) {
        LOG_MSG(warning, "No arguments provided. Pass the name of the executable you want to inject into as an argument.\n");
        system("pause");
        return 1;
    }
    if (!path_has_extension(target_name, ".exe")) {
        LOG_MSG(error, "Invalid input. Enter the name of an executable.\n");
        return 1;
    }

    char exe_name[MAX_PATH] = {0};
    path_get_filename(target_name, exe_name);
    if (strlen(exe_name) == 0) {
        LOG_MSG(error, "Failed to get a filename from \"%s\"\n", target_name);
        return 1;
    }

    uint32_t process_id = get_pid_by_name(exe_name);
    if (process_id == 0) {
        LOG_MSG(warning, "Couldn't find a process named \"%s\"\n", exe_name);

        // Try to start up the target
        if (shell_cmd != NULL) {
            LOG_MSG(info, "Running custom command \"%s\"\n", shell_cmd);
            system(shell_cmd);
        }
        else {
            LOG_MSG(info, "Trying to launch on command-line...\n");

            // We need to do "start [EXE]" because otherwise we'll wait for 
            // the launched process to exit.
            char cmd[2048] = {0}; // TODO: What if someone has a crazy long cmd?
            snprintf(cmd, sizeof(cmd), "start %s", target_name); // target_name may have full path.
            system(cmd);
        }

        process_id = get_pid_by_name(exe_name);
        if (process_id == 0) {
            LOG_MSG(error, "Couldn't find a process named \"%s\"\n", exe_name);
            return 1;
        }
    }

    LOG_MSG(info, "Injecting into %s\n", exe_name);
    self_inject(process_id, bootstrap);

    return 0;
}

