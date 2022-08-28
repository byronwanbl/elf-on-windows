#include "elf_on_windows.hpp"

namespace elf_on_windows
{
CygwinDLL::CygwinDLL()
  : cygwin1_dll("cygwin1.dll")
  , cygstdcxx_6_dll("cygstdc++-6.dll")
  , cyggcc_s_seh_1_dll("cyggcc_s-seh-1.dll")
{
}

void CygwinDLL::link_to(ElfFile& file)
{
    file.dynamic_link(cygwin1_dll);
    file.dynamic_link(cygstdcxx_6_dll);
    file.dynamic_link(cyggcc_s_seh_1_dll);
}

void CygwinDLL::init() { auto init_func = cygwin1_dll.get_symbol("cygwin_dll_init"); }

CygwinDLL::~CygwinDLL() { }
}