#include <cassert>
#include <cstdio>

#include "debug.hpp"
#include "elf_on_windows.hpp"
#include "extra_function.hpp"

namespace elf_on_windows
{
ElfFile* current_elf_file;
Environment* global_env;
MsysDLL* global_msys_dll;

Environment::Environment(int argc, char** argv)
  : argc(argc)
  , argv(argv)
{
    env_var_raw = (char**)malloc(sizeof(char*));
    env_var_raw[0] = NULL;
}

Environment::~Environment() { free(env_var_raw); }

void ElfFile::exec_with_msys(Environment* env, MsysDLL* msys)
{
    current_elf_file = this;
    global_env = env;

    info() << "Execute: " << hex(ehdr.e_entry) << endl;
    __asm__("jmp *%0 \n\t" : : "r"(mem_img.base + ehdr.e_entry) : "cc", "memory");
}
}