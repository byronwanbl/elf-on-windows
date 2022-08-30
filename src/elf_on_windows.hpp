#ifndef ELF_ON_WINDOWS_HPP
#define ELF_ON_WINDOWS_HPP

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "elf.h"

namespace elf_on_windows
{
struct Range
{
    uint64_t begin, size;
};

class ElfSymbol;
class ElfRela;
class Environment;
class WindowsLibrary;

class ElfFile;
class MsysDLL;

class ElfMemImage
{
public:
    static const uint64_t AUTO_BASE = 0xffffffffffffffff;

    uint64_t base;
    uint64_t alloc_begin, alloc_size;
    std::vector<Range> using_memory;

    ElfMemImage();
    void init(ElfFile&);
    ~ElfMemImage();

    bool exist(uint64_t);
    uint64_t fixed_addr(uint64_t);
    uint64_t& get_fixed(uint64_t);

private:
    void alloc();
    void load(ElfFile&);

    void free();
};

class ElfFile
{
public:
    std::string filename;
    std::ifstream in;

    Elf64_Ehdr ehdr;
    std::vector<Elf64_Phdr> phdr;
    std::vector<Elf64_Dyn> dyn_list;

    std::vector<uint64_t> required_id;
    std::vector<std::string> require_string;

    uint64_t string_table_addr = 0, string_table_size = 0;
    std::map<uint32_t, std::string> string_table;

    uint64_t symbol_table_addr = 0;
    std::vector<Elf64_Sym> symbol_table_raw;
    std::vector<ElfSymbol> symbol_table;

    uint64_t rela_plt_addr = 0, rela_plt_size = 0;
    uint64_t rela_dyn_addr = 0, rela_dyn_size = 0;
    std::vector<Elf64_Rela> rela_dyn_raw, rela_plt_raw;
    std::vector<ElfRela> rela;

    uint64_t init_1 = 0, init_arr_addr = 0, init_arr_size = 0;
    std::vector<uint64_t> init_arr;
    uint64_t fini_1 = 0, fini_arr_addr = 0, fini_arr_size = 0;
    std::vector<uint64_t> fini_arr;

    ElfMemImage mem_img;

    ElfFile(const std::string&);
    ~ElfFile();

    void pre_dynamic_link();
    void dynamic_link(WindowsLibrary&);
    void dynamic_link_func(const std::string&, uint64_t, bool force=false);
    void dynamic_link_variable(const std::string&, uint64_t, bool force=false);
    void check_dynamic_link();

    void dynamic_link_extra_func();
    // void dynamic_link(ElfFile);

    void exec_with_msys(Environment*, MsysDLL*);

private:
    void load_header();
    void load_dyn_info();
};

class ElfSymbol
{
public:
    std::string name;
    uint64_t size;
    uint64_t value;

    enum struct Type
    {
        NOTYPE,
        OBJECT,
        FUNC,
        SECTION,
        FILE,
        LO_PROC,
        HI_PROC
    } type;
    enum struct Bind
    {
        LOCAL,
        GLOBAL,
        WEAK,
        LO_PROC,
        HI_PROC
    } bind;

    ElfSymbol(const ElfFile&, Elf64_Sym);
};

class ElfRela
{
public:
    const ElfSymbol& sym;
    uint64_t offset, addend;
    enum struct Type
    {
        COPY = 5,
        GLOBAL_DAT = 6,
        JUMP_SLOT = 7
    } type;

    ElfRela(const ElfFile&, Elf64_Rela);
};

class Environment
{
public:
    int argc;
    char** argv;
    char** env_var_raw;

    Environment(int, char**);
    ~Environment();
};

class WindowsLibrary
{
public:
    std::string filename;
    void* module;

    WindowsLibrary(const std::string&);
    ~WindowsLibrary();
    uint64_t get_symbol(const std::string&);
};

class MsysDLL
{
public:
    WindowsLibrary libc;
    WindowsLibrary libstdcxx;
    WindowsLibrary libgcc;

    MsysDLL();
    void link_to(ElfFile&);
    ~MsysDLL();
};

uint64_t generate_call_wrapper_linux_to_windows(uint64_t, uint64_t, uint64_t);
uint64_t call_linux_func(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
}

#endif