#include <cassert>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"
#include "util.hpp"

namespace elf_on_windows
{
void ElfFile::pre_dynamic_link()
{
    for (auto rela : rela_dyn) {
        if (rela.type == ElfRela::Type::GLOBAL_DAT && mem_img.exist(rela.offset))
            mem_img.get_fixed(rela.offset) = 0;
    }
    for (auto rela : rela_plt) {
        if (rela.type == ElfRela::Type::JUMP_SLOT && mem_img.exist(rela.offset))
            mem_img.get_fixed(rela.offset) = 0;
    }
}

void ElfFile::dynamic_link(WindowsLibrary& lib)
{
    for (auto rela : rela_dyn) {
        if (rela.type == ElfRela::Type::GLOBAL_DAT && mem_img.exist(rela.offset)) {
            auto proc = lib.get_symbol(rela.sym.name);
            if (proc) {
                mem_img.get_fixed(rela.offset) = proc;
                debug() << "Linked " << rela.sym.name << " from " << lib.filename << endl;
            }
        }
    }

    for (auto rela : rela_plt) {
        if (rela.type == ElfRela::Type::JUMP_SLOT && mem_img.exist(rela.offset)) {
            auto proc = lib.get_symbol(rela.sym.name);
            if (proc) {
                mem_img.get_fixed(rela.offset) = proc;
                debug() << "Linked " << rela.sym.name << " from " << lib.filename << endl;
            }
        }
    }
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