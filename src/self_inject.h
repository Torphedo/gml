#pragma once
#include <stdbool.h>
#include <stdint.h>

// This just lets us avoid including all of windows.h in this header.
typedef unsigned long (* __stdcall CUSTOM_THREAD_START_ROUTINE)(void*);

/// Get the ID of a process by searching for its main executable's name
/// \param process_name The name of the main executable of the process
/// \return Windows process ID, or 0 on failure
uint32_t get_pid_by_name(const char* process_name);

/// Inject the current executable into another process and call the specified
/// function using CreateRemoteThread().
/// \param process_id ID of the process you want to inject into
/// \param entry_point Function pointer to the start routine you want to use.
/// THIS NEEDS TO BE A REAL LPTHREAD_START_ROUTINE OR IT WILL CRASH ON RETURN!
/// \return boolean success/failure.
bool self_inject(uint32_t process_id, CUSTOM_THREAD_START_ROUTINE entry_point);

