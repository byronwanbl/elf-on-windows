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
class WindowsLibrary;

class ElfFile;

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

    uint64_t string_table_off, string_table_size;
    std::map<uint32_t, std::string> string_table;

    uint64_t symbol_table_off;
    std::vector<Elf64_Sym> symbol_table_raw;
    std::vector<ElfSymbol> symbol_table;

    uint64_t rela_plt_off, rela_plt_size;
    uint64_t rela_dyn_off, rela_dyn_size;
    std::vector<Elf64_Rela> rela_dyn_raw, rela_plt_raw;
    std::vector<ElfRela> rela_dyn, rela_plt;

    ElfMemImage mem_img;

    ElfFile(std::string filename);
    ~ElfFile();

    void test_dynamic_link();
    // void dynamic_link(WindowsLibrary);
    // void dynamic_link(ElfFile);

    void exec();
private:
    void load_header();
    void load_dyn_info();
};

class ElfSymbol
{
public:
    std::string name;

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
    uint32_t type;

    ElfRela(const ElfFile&, Elf64_Rela);
};


class WindowsLibrary
{
    void* handle;
};
}

#endif