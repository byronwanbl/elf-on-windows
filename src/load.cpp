#include <cassert>
#include <cstring>

#include "debug.hpp"
#include "elf_on_windows.hpp"
#include "util.hpp"

namespace elf_on_windows
{
void _check_elf_format(Elf64_Ehdr& ehdr);

ElfFile::ElfFile(const std::string& filename)
  : filename(filename)
{
    info() << "Load file: " << filename << endl;
    in.open(filename, std::ios::binary);
    if (!in) {
        error() << "Cannot open file: " << filename << endl;
        panic();
    }

    load_header();
    mem_img.init(*this);
    load_dyn_info();
}

ElfFile::~ElfFile() { in.close(); }

void ElfFile::load_header()
{
    debug() << "Load headers" << endl;

    in.seekg(0);
    in.read((char*)&ehdr, sizeof(ehdr));
    _check_elf_format(ehdr);

    phdr.resize(ehdr.e_phnum);
    assert(ehdr.e_phentsize == sizeof(Elf64_Phdr));
    in.seekg(ehdr.e_phoff);
    in.read((char*)phdr.data(), ehdr.e_phnum * ehdr.e_phentsize);
}

void ElfFile::load_dyn_info()
{
    debug() << "Load dynamic info" << endl;
    for (auto p : phdr) {
        if (p.p_type == PT_DYNAMIC) {
            auto dyn_ptr = (Elf64_Dyn*)mem_img.fixed_addr(p.p_vaddr);
            Elf64_Dyn dyn;
            do {
                dyn = *dyn_ptr;
                dyn_list.push_back(dyn);
                dyn_ptr++;
            } while (dyn.d_tag != DT_NULL);
        }
    }

    debug() << "Load dynamic section" << endl;
    for (auto dyn : dyn_list) {
        uint64_t val = dyn.d_un.d_val;
        switch (dyn.d_tag) {
            case DT_NEEDED: required_id.push_back(val); break;
            case DT_STRTAB: string_table_addr = val; break;
            case DT_SYMTAB: symbol_table_addr = val; break;
            case DT_STRSZ: string_table_size = val; break;
            case DT_SYMENT: assert(val == sizeof(Elf64_Sym)); break;
            case DT_PLTRELSZ: rela_plt_size = val; break;
            case DT_PLTREL: /*assert(...);*/ break;
            case DT_JMPREL: rela_plt_addr = val; break;
            case DT_RELA: rela_dyn_addr = val; break;
            case DT_RELAENT: assert(val == sizeof(Elf64_Rela)); break;
            case DT_RELASZ: rela_dyn_size = val; break;
            case DT_INIT: init_1 = val; break;
            case DT_INIT_ARRAY: init_arr_addr = val; break;
            case DT_INIT_ARRAYSZ: init_arr_size = val; break;
            case DT_FINI: fini_1 = val; break;
            case DT_FINI_ARRAY: fini_arr_addr = val; break;
            case DT_FINI_ARRAYSZ: fini_arr_size = val; break;
        }
    }

    debug() << "Load string table: " << hex(string_table_addr) << endl;
    auto string_table_ptr = (char*)mem_img.fixed_addr(string_table_addr);
    while (true) {
        auto off = (uint64_t)string_table_ptr - mem_img.fixed_addr(string_table_addr);
        if (off >= string_table_size) break;
        std::string s(string_table_ptr);
        string_table[off] = s;
        string_table_ptr += strlen(string_table_ptr) + 1;
    }
    for (auto req : required_id)
        require_string.push_back(string_table[req]);

    debug() << "Load symbol table: " << hex(symbol_table_addr) << endl;
    auto symbol_table_ptr = (Elf64_Sym*)mem_img.fixed_addr(symbol_table_addr);
    do {
        Elf64_Sym sym;
        sym = *symbol_table_ptr;
        symbol_table_raw.push_back(sym);
        symbol_table_ptr += 1;
    } while (*((char*)symbol_table_ptr) != 0);
    for (auto sym : symbol_table_raw)
        symbol_table.push_back(ElfSymbol(*this, sym));

    debug() << "Load rela info" << endl;
    rela_dyn_raw.resize(rela_dyn_size / sizeof(Elf64_Rela));
    std::copy((uint8_t*)mem_img.fixed_addr(rela_dyn_addr),
      (uint8_t*)mem_img.fixed_addr(rela_dyn_addr + rela_dyn_size),
      (uint8_t*)rela_dyn_raw.data());
    for (auto r : rela_dyn_raw)
        rela.push_back(ElfRela(*this, r));

    rela_plt_raw.resize(rela_plt_size / sizeof(Elf64_Rela));
    std::copy((uint8_t*)mem_img.fixed_addr(rela_plt_addr),
      (uint8_t*)mem_img.fixed_addr(rela_plt_addr + rela_plt_size),
      (uint8_t*)rela_plt_raw.data());
    for (auto r : rela_plt_raw)
        rela.push_back(ElfRela(*this, r));

    debug() << "Load `init_array` and `fini_array`" << endl;
    debug() << "`init_array`: " << range(init_arr_addr, init_arr_size) << endl;
    init_arr.resize(init_arr_size / sizeof(void*));
    std::copy((uint8_t*)mem_img.fixed_addr(init_arr_addr),
      (uint8_t*)mem_img.fixed_addr(init_arr_addr + init_arr_size),
      (uint8_t*)init_arr.data());
    debug() << "`fini_array`: " << range(fini_arr_addr, fini_arr_size) << endl;
    fini_arr.resize(fini_arr_size / sizeof(void*));
    std::copy((uint8_t*)mem_img.fixed_addr(fini_arr_addr),
      (uint8_t*)mem_img.fixed_addr(fini_arr_addr + fini_arr_size),
      (uint8_t*)fini_arr.data());
    debug() << "Load completed" << endl;
}

ElfSymbol::ElfSymbol(const ElfFile& file, Elf64_Sym sym)
{
    if (!file.string_table.count(sym.st_name)) {
        warning() << "Cannot find string indexed " << hex(sym.st_name) << ", skipped" << endl;
        name = "<UNKNOWN>";
        return;
    }
    name = file.string_table.find(sym.st_name)->second;
    type = (ElfSymbol::Type)ELF64_ST_TYPE(sym.st_info);
    bind = (ElfSymbol::Bind)ELF64_ST_BIND(sym.st_info);
    size = sym.st_size;
    value = sym.st_value;
}

ElfRela::ElfRela(const ElfFile& file, Elf64_Rela rela)
  : sym(file.symbol_table[ELF64_R_SYM(rela.r_info)])
{
    offset = rela.r_offset;
    addend = rela.r_addend;
    type = (ElfRela::Type)ELF64_R_TYPE(rela.r_info);
    if (ELF64_R_SYM(rela.r_info) >= file.symbol_table.size()) {
        type = (ElfRela::Type)-1;
    }
}

void _check_elf_format(Elf64_Ehdr& ehdr)
{
    for (int i = 0; i < 4; i++)
        if (ehdr.e_ident[i] != ELFMAG[i]) {
            error() << "`elf magic` does not match." << endl;
            panic();
        }
    if (ehdr.e_ident[4] != ELFCLASS64) {
        error() << "Must be 64-bit elf file." << endl;
        panic();
    }
    if (ehdr.e_ident[5] != ELFDATA2LSB) {
        error() << "Must be little endian." << endl;
        panic();
    }
    assert(ehdr.e_ident[6] == EV_CURRENT);
    if (ehdr.e_type != ET_EXEC && ehdr.e_type != ET_DYN) {
        error() << "Only support executable file and dynamic file." << endl;
        panic();
    }
}
}