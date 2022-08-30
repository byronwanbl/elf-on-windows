#include "elf_on_windows.hpp"

int main(int argc, char** argv)
{
    elf_on_windows::ElfFile file("test/sample2");
    elf_on_windows::MsysDLL msys;
    elf_on_windows::Environment env((size_t)argc, argv);
    file.pre_dynamic_link();
    msys.link_to(file);
    file.dynamic_link_extra_func();
    file.check_dynamic_link();
    file.exec_with_msys(&env, &msys);

    return 0;
}
