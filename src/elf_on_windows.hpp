#ifndef ELF_ON_WINDOWS_H
#define ELF_ON_WINDOWS_H

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "elf.h"

namespace elf_on_windows
{

class ElfSymbol;
class ElfRela;

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

    ElfFile(std::string filename);
    ~ElfFile();

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
}

#endif