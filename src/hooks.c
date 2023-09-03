// Reduce the size of Windows.h to improve compile time
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMB
#include <windows.h>
#include <stdio.h>

#include <MinHook.h>

#include "hooks.h"
#include "logging.h"

typedef FILE* (*FOPEN)(const char*, const char*);
FOPEN original_fopen = NULL;

typedef HANDLE (*create_file_a)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
create_file_a original_CreateFileA = NULL;

typedef HANDLE (*create_file_w)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
create_file_w original_CreateFileW = NULL;

FILE* hook_fopen(const char* name, const char* stream_type) {
    return original_fopen("C:\\storage\\dev\\git\\gml\\redirect.txt", stream_type);
}

HANDLE WINAPI hook_CreateFileA (LPCSTR filename, DWORD access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_info, DWORD creation_flags, DWORD file_attributes, HANDLE template_file) {
    return original_CreateFileA("C:\\storage\\dev\\git\\gml\\redirect.txt", access, share_mode, security_info, creation_flags, file_attributes, template_file);
}

HANDLE WINAPI hook_CreateFileW (LPCWSTR lpFileName, DWORD access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_info, DWORD creation_flags, DWORD file_attributes, HANDLE template_file) {
    return original_CreateFileW(L"C:\\storage\\dev\\git\\gml\\redirect.txt", access, share_mode, security_info, creation_flags, file_attributes, template_file);
}

bool hook_io() {
    if (MH_Initialize() != MH_OK) {
        LOG_MSG(error, "Failed to initialize MinHook\n");
        return false;
    }

  	LOG_MSG(info, "Creating file hooks...\n");
    MH_CreateHookApi(L"msvcrt.dll", "fopen", &hook_fopen, (void**)&original_fopen);
  	LOG_MSG(info, "fopen() hooked\n");
    MH_CreateHookApi(L"KERNEL32.dll", "CreateFileA", &hook_CreateFileA, (void**)&original_CreateFileA);
    MH_CreateHookApi(L"KERNEL32.dll", "CreateFileW", &hook_CreateFileW, (void**)&original_CreateFileW);
	LOG_MSG(info, "Created file hooks\n");

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        LOG_MSG(error, "Failed to enable all hooks\n");
        return false;
    }

    return true;
}

