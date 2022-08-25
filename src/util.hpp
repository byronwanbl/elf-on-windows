#ifndef ELF_ON_WINDOWS_UTIL_H
#define ELF_ON_WINDOWS_UTIL_H

#include "debug.hpp"
#include <cstdlib>

namespace elf_on_windows
{
void panic()
{
    cerr << "Panic!" << endl;
    exit(-1);
}
}

#endif