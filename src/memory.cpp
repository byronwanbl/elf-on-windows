#include <cassert>
#include <fstream>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"
#include "util.hpp"

namespace elf_on_windows
{
ElfMemImage::ElfMemImage() { }

void ElfMemImage::init(ElfFile& file)
{
    if (file.ehdr.e_type == ET_DYN) base = AUTO_BASE;
    for (auto phdr : file.phdr) {
        if (phdr.p_type == PT_LOAD) using_memory.push_back({ phdr.p_vaddr, phdr.p_memsz });
    }

    bool first_flag = true;
    for (auto mem : using_memory) {
        if (first_flag) {
            alloc_begin = mem.begin;
            alloc_size = mem.size;
            first_flag = false;
        } else {
            alloc_size = std::max(alloc_begin + alloc_size, mem.begin + mem.size) -
                         std::min(alloc_begin, mem.begin);
            alloc_begin = std::min(alloc_begin, mem.begin);
        }
    }

    alloc();
    load(file);
}

void ElfMemImage::alloc()
{
    if (base == AUTO_BASE) {
        assert(alloc_begin == 0);
        info() << "Alloc virtual memory: (automatic address) " << range(alloc_begin, alloc_size)
               << endl;
        base = (uint64_t)VirtualAlloc(
          nullptr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (base == 0) {
            error() << "Alloc virtual memory failed, GetLastError() = " << GetLastError() << endl;
            panic();
        }
    } else {
        info() << "Alloc virtual memory: " << range(alloc_begin, alloc_size) << endl;
        auto addr = VirtualAlloc(
          (void*)alloc_begin, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (addr == nullptr) {
            error() << "Alloc virtual memory failed, GetLastError() = " << GetLastError() << endl;
            panic();
        }
    }
}

void ElfMemImage::load(ElfFile& file)
{
    for (auto phdr : file.phdr) {
        if (phdr.p_type == PT_LOAD) {
            info() << "Load " << hex(phdr.p_offset) << " -> " << range(phdr.p_vaddr, phdr.p_memsz)
                   << endl;
            file.in.seekg(phdr.p_offset);
            file.in.read((char*)phdr.p_vaddr + base, phdr.p_filesz);
        }
    }
}

ElfMemImage::~ElfMemImage() { free(); }

void ElfMemImage::free() { VirtualFree((void*)(base == 0 ? alloc_begin : base), 0, MEM_RELEASE); }
}