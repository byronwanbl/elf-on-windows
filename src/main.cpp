#include "elf_on_windows.hpp"

int main(int argc, char** argv)
{
    elf_on_windows::ElfFile file("sample2");
    elf_on_windows::CygwinDLL cygwin;
    cygwin.link_to(file);
    file.exec();

    return 0;
}
