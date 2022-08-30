#include "elf_on_windows.hpp"

int main(int argc, char** argv)
{
    std::string filename = "test/lambda-hs-exe";
    if (argc > 1) {
        filename = argv[1];
    }

    elf_on_windows::ElfFile file(filename);
    elf_on_windows::MsysDLL msys;
    elf_on_windows::Environment env((size_t)argc - 1, argv + 1);

    file.pre_dynamic_link();
    msys.link_to(file);
    file.dynamic_link_extra_func();
    file.check_dynamic_link();
    file.exec_with_msys(&env, &msys);

    return 0;
}
