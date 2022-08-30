#include <cassert>
#include <cstdio>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"

namespace elf_on_windows
{
uint8_t call_wrapper_linux_to_windows[] = {
#include "call_wrapper_linux_to_windows.hex"
};

uint64_t inject(uint8_t* addr, size_t size, uint64_t old, uint64_t val)
{
    for (int i = 0; i + 8 < size; i++) {
        if (*(uint64_t*)(addr + i) == old) {
            *(uint64_t*)(addr + i) = val;
            return true;
        }
    }
    return false;
}

uint64_t generate_call_wrapper_linux_to_windows(
  uint64_t target_func, uint64_t linux_virt_addr, uint64_t linux_virt_size)
{
    debug() << "Generate calling convention wrapper for " << hex(target_func) << endl;
    size_t size = sizeof(call_wrapper_linux_to_windows);

    uint8_t* virt_addr =
      (uint8_t*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    debug() << "Wrapped placed at " << hex((uint64_t)virt_addr) << endl;
    std::copy(call_wrapper_linux_to_windows, call_wrapper_linux_to_windows + size, virt_addr);

    assert(inject(virt_addr, size, 0x0000000000000000, target_func));
    assert(inject(virt_addr, size, 0x0101010101010101, linux_virt_addr));
    assert(inject(virt_addr, size, 0x0202020202020202, linux_virt_addr + linux_virt_size));
    assert(inject(virt_addr, size, 0x0303030303030303, target_func));

    return (uint64_t)virt_addr;
}

uint64_t call_linux_func(
  uint64_t addr, uint64_t a0 = 0, uint64_t a1 = 0, uint64_t a2 = 0, uint64_t a3 = 0)
{
    uint64_t ret;
    __asm__(
      "pushq %%rbp \n\t"
      "movq %%rsp, %%rbp \n\t"
      "andq $~0x0f, %%rsp \n\t"
      "callq *%%rax \n\t"
      "movq %%rbp, %%rsp \n\t"
      "popq %%rbp \n\t"
      : "=a"(ret)
      : "a"(addr), "D"(a0), "S"(a1), "d"(a2), "c"(a3)
      : "cc", "memory");
    return ret;
}
}