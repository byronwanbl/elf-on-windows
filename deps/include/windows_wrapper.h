// Fake windows.h
// For code analyse

#ifndef CODE_ANALYSE

#include <windows.h>

#else

#ifndef ELF_ON_WINDOWS_WINDOWS_WRAPPER_H
#define ELF_ON_WINDOWS_WINDOWS_WRAPPER_H

#include <cstdint>
#include <cstdio>

#define __stdcall

// https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
// https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection-constants

void* VirtualAlloc(void* lpAddress, size_t dwSize, uint32_t flAllocationType, uint32_t flProtect);
bool VirtualFree(void* lpAddress, size_t dwSize, uint32_t dwFreeType);


const uint32_t MEM_COMMIT = 0x00001000;
const uint32_t MEM_RESERVE = 0x00002000;
const uint32_t MEM_RELEASE = 0x00008000;

const uint32_t PAGE_EXECUTE_READWRITE = 0x40;


// https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror

uint32_t GetLastError();

typedef void* HMODULE;

HMODULE LoadLibraryA(const char* lpLibFileName);
bool FreeLibrary(HMODULE hLibModule);
void* GetProcAddress(HMODULE hModule, const char* lpProcName);
#endif

#endif