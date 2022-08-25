#ifndef ELF_ON_WINDOWS_UTIL_HPP
#define ELF_ON_WINDOWS_UTIL_HPP

#include <exception>

#include "debug.hpp"

namespace elf_on_windows
{
class serious_fault : public std::exception
{
};

static void panic()
{
    cerr << "Panic!" << endl;
    throw serious_fault {};
}
}

#endif