#include "elf_on_windows.hpp"

namespace elf_on_windows
{
MsysDLL::MsysDLL()
  : libc("msys-2.0.dll")
  , libstdcxx("msys-stdc++-6.dll")
  , libgcc("msys-gcc_s-seh-1.dll")
{
}

void MsysDLL::link_to(ElfFile& file)
{
    file.dynamic_link(libc);
    file.dynamic_link(libstdcxx);
    file.dynamic_link(libgcc);
}

MsysDLL::~MsysDLL() { }
}