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
        auto func = current_elf_file->mem_img.fixed_addr(init);
        fprintf(stderr, "Call init function: 0x%llx\n", (uint64_t)func);
        call_linux_func(
          func, global_env->argc, (uint64_t)global_env->argv, (uint64_t)global_env->env_var_raw, 0);
    }
    for (auto fini : current_elf_file->fini_arr) {
        auto func = current_elf_file->mem_img.fixed_addr(fini);
        atexit((void (*)())func);
    }
    fprintf(stderr, "Call function main: 0x%llx\n", (uint64_t)main);
    int ret = call_linux_func(main, global_env->argc, (uint64_t)global_env->argv, 0, 0);
    fprintf(stderr, "Function main returned: %d", ret);
    exit(ret);
}

void null_function() { }

bool libc_single_threaded() { return false; }

void* return_stdin() { return stdin; }
void* return_stdout() { return stdout; }
void* return_stderr() { return stderr; }

void assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    fprintf(stderr, "%s: line %d, function %s: %s", file, line, function, assertion);
    exit(-1);
}

thread_local int _errno;
int32_t* errno_location() {
    return &_errno;
}
}

void ElfFile::dynamic_link_extra_func()
{
    using namespace extra_function_dynamic_linking;
    dynamic_link_func("__libc_start_main", (uint64_t)libc_start_main);
    dynamic_link_func("__gmon_start__", (uint64_t)null_function);
    dynamic_link_func("__libc_single_threaded", (uint64_t)libc_single_threaded);
    dynamic_link_func("__isoc99_scanf", (uint64_t)scanf);
    dynamic_link_func("__isoc99_sscanf", (uint64_t)sscanf);
    dynamic_link_func("stdin", (uint64_t)return_stdin);
    dynamic_link_func("stdout", (uint64_t)return_stdout);
    dynamic_link_func("stderr", (uint64_t)return_stderr);
    dynamic_link_func("__assert_fail", (uint64_t)assert_fail);
    dynamic_link_func("__errno_location", (uint64_t)errno_location);
}
}