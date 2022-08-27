#include "elf_on_windows.hpp"

int main(int argc, char** argv)
{
    elf_on_windows::ElfFile file("sample");
    file.test_dynamic_link();
    file.exec();

    return 0;
}
