#include <cassert>

#include "windows_wrapper.h"

#include "debug.hpp"
#include "elf_on_windows.hpp"

namespace elf_on_windows
{
uint64_t generate_calling_convention_wrapper(uint64_t);

namespace test_function
{
void __libc_start_main(int (*main)())
{
    exit(main());
}


void puts(const char* str) { printf("%s", str); }
}

void ElfFile::test_dynamic_link()
{
    warning() << "Use test dynamic link." << endl;
    std::map<std::string, uint64_t> func_map;

    func_map["__libc_start_main"] =
      generate_calling_convention_wrapper((uint64_t)test_function::__libc_start_main);
    func_map["puts"] = generate_calling_convention_wrapper((uint64_t)test_function::puts);

    for (auto rela : rela_dyn) {
        if (func_map[rela.sym.name])
            *(uint64_t*)(mem_img.base + rela.offset) = func_map[rela.sym.name];
    }
    for (auto rela : rela_plt) {
        if (func_map[rela.sym.name])
            *(uint64_t*)(mem_img.base + rela.offset) = func_map[rela.sym.name];
    }
}

uint8_t calling_convention_covert_asm[] = {
#include "calling_convention_convert.hex"
};

uint64_t generate_calling_convention_wrapper(uint64_t target_func)
{
    debug() << "Generate calling convention wrapper for " << hex(target_func) << endl;
    size_t len = sizeof(calling_convention_covert_asm);

    uint8_t* virt_addr =
      (uint8_t*)VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    debug() << "Wrapped placed at " << hex((uint64_t)virt_addr) << endl;
    std::copy(calling_convention_covert_asm, calling_convention_covert_asm + len, virt_addr);

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
}