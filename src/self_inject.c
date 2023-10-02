#include "logging.h"
#include <stdlib.h>

// Reduce the size of Windows.h to improve compile time
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOMB
#include <windows.h>
// For process snapshots.
#include <tlhelp32.h>

#include "self_inject.h"


uint32_t get_pid_by_name(const char* process_name) {
    PROCESSENTRY32 entry = {
            .dwSize = sizeof(PROCESSENTRY32)
    };

    // Capture a list of all running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Loop through processes, do string comparisons until we find it or fail.
    if (Process32First(snapshot, &entry)) {
        while (Process32Next(snapshot, &entry)) {
            if (!lstrcmpi(entry.szExeFile, process_name)) {
                // Success. Clean up and return the PID.
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }

    // Failure. Clean up and return 0
    CloseHandle(snapshot);
    return 0;
}

IMAGE_SECTION_HEADER get_section_header(const char* name, uint8_t* dll_data) {
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)dll_data;
	IMAGE_NT_HEADERS* nt_header =  (IMAGE_NT_HEADERS*)(dll_data + dos_header->e_lfanew);

	// Get section headers
	uint32_t section_header_rva = dos_header->e_lfanew + sizeof(*nt_header);
	IMAGE_SECTION_HEADER* headers = (IMAGE_SECTION_HEADER*)(dll_data + section_header_rva);

	// Search through the sections for our target
	for (uint32_t i = 0; i < nt_header->FileHeader.NumberOfSections; i++) {
		char* cur_name = (char*)headers[i].Name;
		if (strncmp(name, cur_name, strlen(name)) == 0) {
			return headers[i];
		}
	}

	// Oh well.
	IMAGE_SECTION_HEADER failure = {0};
	return failure;
}

typedef struct BASE_RELOCATION_ENTRY {
    USHORT Offset : 12;
    USHORT Type : 4;
}BASE_RELOCATION_ENTRY;

bool self_inject(uint32_t process_id, LPTHREAD_START_ROUTINE entry_point) {
    // Open the target process we'll be injecting this PE into
    DWORD access = PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION;
    HANDLE target_process = OpenProcess(access, FALSE, process_id);
    if (target_process == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Get current image's base address
    uint8_t* image_base = (uint8_t*) GetModuleHandle(NULL);
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*) image_base;
    IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*) (image_base + dos_header->e_lfanew);

    // Allocate buffers for the image in target and current process
    uint8_t* local_image = malloc(nt_header->OptionalHeader.SizeOfImage);
    uint8_t* target_image = VirtualAllocEx(target_process, NULL, nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    // Early exit if we fail an allocation.
    if (local_image == NULL || target_image == NULL) {
        free(local_image);
        free(target_image);
        CloseHandle(target_process);
        return false;
    }

    // Copy PE data into our local buffer
    memcpy(local_image, image_base, nt_header->OptionalHeader.SizeOfImage);
    IMAGE_DATA_DIRECTORY* data_dir = nt_header->OptionalHeader.DataDirectory;

    // Move our header pointers to the new image
    /*
    dos_header = (IMAGE_DOS_HEADER*)local_image;
    nt_header = (IMAGE_NT_HEADERS*)local_image + dos_header->e_lfanew;
    */

    IMAGE_SECTION_HEADER reloc_section = get_section_header(".reloc", local_image);

    // Calculate difference between where the image was originally loaded and the target location
    uintptr_t reloc_bias = (uintptr_t)target_image - (uintptr_t)image_base;

    // Relocate local_image, to ensure that it will have correct addresses once it's in the target process
    int32_t reloc_offset = reloc_section.VirtualAddress;
    LOG_MSG(debug, "reloc_offset = 0x%x\n", reloc_offset);

    IMAGE_BASE_RELOCATION* reloc_table = (IMAGE_BASE_RELOCATION*)(local_image + reloc_offset);

    while (reloc_table->SizeOfBlock > 0) {
        // BASE_RELOCATION_ENTRY is 2 bytes, so the size (minus header) over 2 is our entry count
        // (not using sizeof(BASE_RELOCATION_ENTRY) because some compilers might add padding or something on bitfields)
        uint32_t entries_count = (reloc_table->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
        BASE_RELOCATION_ENTRY* entries = (BASE_RELOCATION_ENTRY*)((uintptr_t)reloc_table + sizeof(IMAGE_BASE_RELOCATION));
        LOG_MSG(debug, "%d entries.\n", entries_count);

        for (uint32_t i = 0; i < entries_count; i++) {
            if (entries[i].Offset) {
                // Get the address of the pointer we need to relocate, then add our image base delta.
                if (entries[i].Type != IMAGE_REL_BASED_DIR64) {
                    LOG_MSG(debug, "Doing relocation wrong at offset %p (type %d)\n", entries[i].Offset, entries[i].Type);
                }
                uintptr_t* patched_address = (uintptr_t*)((uintptr_t)local_image + reloc_table->VirtualAddress + entries[i].Offset);
                *patched_address += reloc_bias;
            }
        }
        // Go to the next relocation table and repeat. There is one table per 4KiB page.
        reloc_table = (IMAGE_BASE_RELOCATION*)((uintptr_t)reloc_table + reloc_table->SizeOfBlock);
    }
    LOG_MSG(info, "Relocation finished.\n");

    // Write the relocated PE into the target process
    WriteProcessMemory(target_process, target_image, local_image, nt_header->OptionalHeader.SizeOfImage, NULL);
    free(local_image);

    // Start the injected PE inside the target process
    CreateRemoteThread(target_process, NULL, 0, (LPTHREAD_START_ROUTINE)((uintptr_t)entry_point + reloc_bias), target_image, 0, NULL);
    CloseHandle(target_process);

    return true;
}
