#include <cassert>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"

namespace elf_on_windows
{
uint8_t call_wrapper_linux_to_windows[] = {
#include "call_wrapper_linux_to_windows.hex"
};

uint64_t generate_call_wrapper_linux_to_windows(uint64_t target_func)
{
    debug() << "Generate calling convention wrapper for " << hex(target_func) << endl;
    size_t len = sizeof(call_wrapper_linux_to_windows);

    uint8_t* virt_addr =
      (uint8_t*)VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    debug() << "Wrapped placed at " << hex((uint64_t)virt_addr) << endl;
    std::copy(call_wrapper_linux_to_windows, call_wrapper_linux_to_windows + len, virt_addr);

    bool injected = false;
    for (int i = 0; i + 8 < len; i++) {
        bool zero_flag = true;
        for (int j = 0; j < 8; j++)
            zero_flag &= virt_addr[i + j] == 0;
        if (zero_flag) {
            *(uint64_t*)(virt_addr + i) = target_func;
            injected = true;
            break;
        }
    }
    assert(injected);

    return (uint64_t)virt_addr;
}

uint64_t call_linux_func(
  uint64_t addr, uint64_t a0 = 0, uint64_t a1 = 0, uint64_t a2 = 0, uint64_t a3 = 0)
{
    uint64_t ret;
    __asm__(
      "pushq %%rbp \n\t"
      "movq %%rsp, %%rbp \n\t"
      "andq $0x0f, %%rsp \n\t"
      "callq *%%rax \n\t"
      "movq %%rbp, %%rsp \n\t"
      : "=a"(ret)
      : "a"(addr), "D"(a0), "S"(a1), "d"(a2), "c"(a3)
      : "cc", "memory");
    return ret;
}
}