#include "windows_wrapper.h"

#include "elf_on_windows.hpp"
#include "debug.hpp"

namespace elf_on_windows
{
void ElfFile::dynamic_link()
{
    warning() << "Use test dynamic link." << endl;
}

void windows_fastcall_to_linux_fastcall()
{

}
}