#include <stdio.h>
#include <stdint.h>

#include <Windows.h>

#include "console.h"

bool console_redirect_stdio() {
    bool result = true;
    FILE *fp;

    void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Redirect STDIN
    if (freopen_s(&fp, "CONIN$", "r", stdin) != 0) {
        result = false;
    } else {
        setvbuf(stdin, NULL, _IONBF, 0);
    }

    // Redirect STDOUT
    if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0) {
        result = false;
    } else {
        setvbuf(stdout, NULL, _IONBF, 0);
    }

    // Redirect STDERR
    if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0) {
        result = false;
    } else {
        setvbuf(stderr, NULL, _IONBF, 0);
    }

    return result;
}

bool enable_ansi_codes() {
    DWORD prev_console_mode;
    void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    GetConsoleMode(console_handle , &prev_console_mode);
    if (prev_console_mode == 0) {
        return false;
    }

    SetConsoleMode(console_handle, prev_console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
    return true;
}

bool console_setup() {
    AllocConsole();
    void* console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Set the screen buffer height for the console
    int16_t min_height = 32000;
    CONSOLE_SCREEN_BUFFER_INFO console_info = {0};
    GetConsoleScreenBufferInfo(console_handle, &console_info);
    if (console_info.dwSize.Y < min_height) {
        console_info.dwSize.Y = min_height;
    }
    SetConsoleScreenBufferSize(console_handle, console_info.dwSize);

    // Show the console window.
    HWND window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, SW_SHOW);

    enable_ansi_codes();
    // Enable UTF-8 unicode. This should make it easier to print special
    // characters like Japanese text.
    uint32_t output_cp = GetConsoleOutputCP();
    if (output_cp != 0 && output_cp != CP_UTF8) {
        SetConsoleOutputCP(CP_UTF8);
    }

    return console_redirect_stdio();
}

