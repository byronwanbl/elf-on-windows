#ifndef ELF_ON_WINDOWS_UTIL_HPP
#define ELF_ON_WINDOWS_UTIL_HPP

#include <exception>

#include "debug.hpp"

namespace elf_on_windows
{
static void panic()
{
    cerr << "Panic!" << endl;
    exit(-1);
}
}

#endif