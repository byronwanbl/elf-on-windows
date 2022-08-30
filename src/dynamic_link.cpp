#include <cassert>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"
#include "util.hpp"

namespace elf_on_windows
{
void ElfFile::pre_dynamic_link()
{
    for (auto r : rela) {
        if ((r.type == ElfRela::Type::GLOBAL_DAT || r.type == ElfRela::Type::JUMP_SLOT) &&
            mem_img.exist(r.offset)) {
            mem_img.get_fixed(r.offset) = 0;
        } else if (r.type == ElfRela::Type::COPY && mem_img.exist(r.offset)) {
            mem_img.get_fixed(r.offset) = r.sym.value;
        }
    }
}

void ElfFile::check_dynamic_link()
{
    for (auto r : rela) {
        if ((r.type == ElfRela::Type::GLOBAL_DAT || r.type == ElfRela::Type::JUMP_SLOT) &&
            mem_img.exist(r.offset)) {
            if (mem_img.get_fixed(r.offset) == 0) {
                warning() << "Does not link " << r.sym.name << endl;
            }
        }
    }
}

void ElfFile::dynamic_link(WindowsLibrary& lib)
{
    debug() << "Link to " << lib.filename << endl;
    for (auto r : rela) {
        if ((r.type == ElfRela::Type::GLOBAL_DAT || r.type == ElfRela::Type::JUMP_SLOT) &&
            mem_img.exist(r.offset)) {
            auto addr = lib.get_symbol(r.sym.name);
            if (addr) {
                debug() << "Link " << r.sym.name << " from " << lib.filename << endl;
                mem_img.get_fixed(r.offset) = generate_call_wrapper_linux_to_windows(
                  addr, mem_img.fixed_addr(mem_img.alloc_begin), mem_img.alloc_size);
            }
        } else if (r.type == ElfRela::Type::COPY && mem_img.exist(r.offset)) {
            auto addr = lib.get_symbol(r.sym.name);
            if (addr) {
                debug() << "Link " << r.sym.name << " , size = " << hex(r.sym.size) << ", from "
                        << lib.filename << endl;
                auto this_addr = mem_img.fixed_addr(r.offset);
                std::copy((uint8_t*)addr, (uint8_t*)(addr + r.sym.size), (uint8_t*)this_addr);
            }
        }
    }
}

void ElfFile::dynamic_link_func(const std::string& name, uint64_t addr, bool force)
{
    for (auto r : rela) {
        if ((r.type == ElfRela::Type::GLOBAL_DAT || r.type == ElfRela::Type::JUMP_SLOT) &&
            mem_img.exist(r.offset) && r.sym.name == name) {
            if (!mem_img.get_fixed(r.offset) || force) {
                debug() << "Link " << r.sym.name << " , using custom function, addr = " << hex(addr)
                        << endl;
                mem_img.get_fixed(r.offset) = generate_call_wrapper_linux_to_windows(
                  addr, mem_img.fixed_addr(mem_img.alloc_begin), mem_img.alloc_size);
                return;
            }
        }
    }

    warning() << "Cannot found symbol " << name << endl;
}

void ElfFile::dynamic_link_variable(const std::string& name, uint64_t addr, bool force)
{
    for (auto r : rela) {
        if (r.type == ElfRela::Type::COPY && mem_img.exist(r.offset) && r.sym.name == name) {
            if (!mem_img.get_fixed(r.offset) || force) {
                debug() << "Link " << r.sym.name << " , using custom function, addr = " << hex(addr)
                        << endl;
                mem_img.get_fixed(r.offset) = addr;
                return;
            }
        }
    }

    warning() << "Cannot found symbol " << name << endl;
}

WindowsLibrary::WindowsLibrary(const std::string& filename)
  : filename(filename)
{
    module = (void*)LoadLibraryA(filename.c_str());
    if (!module) {
        error() << "Cannot load library " << filename << ": " << GetLastError() << endl;
        panic();
    }
}

uint64_t WindowsLibrary::get_symbol(const std::string& symbol)
{
    return (uint64_t)GetProcAddress((HMODULE)module, symbol.c_str());
}

WindowsLibrary::~WindowsLibrary() { FreeLibrary((HMODULE)module); }
}