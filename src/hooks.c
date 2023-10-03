#include <stdio.h>
#include <stdint.h>
// Reduce the size of Windows.h to improve compile time
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMB
#include <windows.h>

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

bool do_api_hook(const wchar_t* dll, const char* func, void* hook, void** original) {
    void* original_addr = NULL;
    uint32_t result = 0;
    result = MH_CreateHookApiEx(dll, func, func, original, &original_addr);
    if (result != MH_OK) {
        LOG_MSG(error, "Failed to place hook for %ls::%s()\n", dll, func);
        return false;
    }
    if (MH_EnableHook(original_addr) != MH_OK) {
        LOG_MSG(error, "Failed to enable hook for %ls::%s()\n", dll, func);
        return false;
    }
    LOG_MSG(info, "Hooked %ls::%s()\n", dll, func);
    return true;
}

bool hook_io() {
    if (MH_Initialize() != MH_OK) {
        LOG_MSG(error, "Failed to initialize MinHook\n");
        return false;
    }

    uint32_t hook_count = 0;
    hook_count += do_api_hook(L"msvcrt.dll", "fopen", &hook_fopen, (void**)&original_fopen);
    hook_count += do_api_hook(L"KERNEL32.dll", "CreateFileA", &hook_CreateFileA, (void**)&original_CreateFileA);
    hook_count += do_api_hook(L"KERNEL32.dll", "CreateFileW", &hook_CreateFileW, (void**)&original_CreateFileW);

    if (hook_count < 3) {
        LOG_MSG(error, "Couldn't apply all hooks.\n");
        return false;
    }

    LOG_MSG(info, "Done.\n");

    return true;
}

