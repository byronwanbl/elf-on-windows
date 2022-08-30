#include "debug.hpp"
#include "elf_on_windows.hpp"

namespace elf_on_windows
{
extern ElfFile* current_elf_file;
extern Environment* global_env;
extern MsysDLL* global_msys_dll;

namespace extra_function_dynamic_linking
{
void libc_start_main(uint64_t main)
{
    for (auto init : current_elf_file->init_arr) {
        auto func = current_elf_file->mem_img.fixed(init);
        call_linux_func(
          func, global_env->argc, (uint64_t)global_env->argv, (uint64_t)global_env->env_var_raw, 0);
    }
    int ret = call_linux_func(main, global_env->argc, (uint64_t)global_env->argv, 0, 0);
    exit(ret);
}

void null_function() { }

bool libc_single_threaded() { return false; }
}
}