#include "debug.hpp"
#include "elf_on_windows.hpp"

namespace elf_on_windows
{
void ElfFile::exec()
{
    info() << "Execute at " << hex(ehdr.e_entry) << endl;
    __asm__(
      "pushq $0 \n\t"
      "pushq $0 \n\t"
      "jmp *%0 \n\t"
      :
      : "r"(mem_img.base + ehdr.e_entry)
      : "cc", "memory");
}
}