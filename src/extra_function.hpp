#ifndef ELF_ON_WINDOWS_EXTRA_FUNCTION_HPP
#define ELF_ON_WINDOWS_EXTRA_FUNCTION_HPP

#include <cstdint>

namespace elf_on_windows
{
namespace extra_function_dynamic_linking
{
void libc_start_main(uint64_t);
void null_function();
bool libc_single_threaded();
}
}

#endif