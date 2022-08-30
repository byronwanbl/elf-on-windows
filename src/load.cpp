#include <cassert>

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
    load_dyn_info();
    mem_img.init(*this);
    load_from_mem();
}

ElfFile::~ElfFile() { in.close(); }

void ElfFile::load_header()
{
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
    for (auto p : phdr) {
        if (p.p_type == PT_DYNAMIC) {
            in.seekg(p.p_offset);
            Elf64_Dyn dyn;
            do {
                in.read((char*)&dyn, sizeof(dyn));
                dyn_list.push_back(dyn);
            } while (dyn.d_tag != DT_NULL);
        }
    }

    for (auto dyn : dyn_list) {
        uint64_t val = dyn.d_un.d_val;
        switch (dyn.d_tag) {
            case DT_NEEDED: required_id.push_back(val); break;
            case DT_STRTAB: string_table_off = val; break;
            case DT_SYMTAB: symbol_table_off = val; break;
            case DT_STRSZ: string_table_size = val; break;
            case DT_SYMENT: assert(val == sizeof(Elf64_Sym)); break;
            case DT_PLTRELSZ: rela_plt_size = val; break;
            case DT_PLTREL: /*assert(...);*/ break;
            case DT_JMPREL: rela_plt_off = val; break;
            case DT_RELA: rela_dyn_off = val; break;
            case DT_RELAENT: assert(val == sizeof(Elf64_Rela)); break;
            case DT_RELASZ: rela_dyn_size = val; break;
            case DT_INIT: init_1 = val;
            case DT_INIT_ARRAY: init_arr_addr = val;
            case DT_INIT_ARRAYSZ: init_arr_size = val;
            case DT_FINI: fini_1 = val;
            case DT_FINI_ARRAY: fini_arr_addr = val;
            case DT_FINI_ARRAYSZ: fini_arr_size = val;
        }
    }

    in.seekg(string_table_off);
    do {
        auto off = (uint64_t)in.tellg() - string_table_off;
        std::string s;
        std::getline(in, s, '\0');
        string_table[off] = s;
    } while (in.tellg() != string_table_off + string_table_size);
    for (auto req : required_id)
        require_string.push_back(string_table[req]);

    in.seekg(symbol_table_off);
    do {
        Elf64_Sym sym;
        in.read((char*)&sym, sizeof(Elf64_Sym));
        symbol_table_raw.push_back(sym);
    } while (in.peek() != 0);
    for (auto sym : symbol_table_raw)
        symbol_table.push_back(ElfSymbol(*this, sym));

    rela_dyn_raw.resize(rela_dyn_size / sizeof(Elf64_Rela));
    in.seekg(rela_dyn_off);
    in.read((char*)rela_dyn_raw.data(), rela_dyn_size);
    for (auto r : rela_dyn_raw)
        rela.push_back(ElfRela(*this, r));

    rela_plt_raw.resize(rela_plt_size / sizeof(Elf64_Rela));
    in.seekg(rela_plt_off);
    in.read((char*)rela_plt_raw.data(), rela_plt_size);
    for (auto r : rela_plt_raw)
        rela.push_back(ElfRela(*this, r));
}

void ElfFile::load_from_mem()
{
    init_arr.resize(init_arr_size / sizeof(void*));
    std::copy(&mem_img.get_fixed(init_arr_addr),
      &mem_img.get_fixed(init_arr_addr + init_arr_size),
      init_arr.data());
    fini_arr.resize(fini_arr_size / sizeof(void*));
    std::copy(&mem_img.get_fixed(fini_arr_addr),
      &mem_img.get_fixed(fini_arr_addr + fini_arr_size),
      fini_arr.data());
}

ElfSymbol::ElfSymbol(const ElfFile& file, Elf64_Sym sym)
{
    if (!file.string_table.count(sym.st_name)) {
        warning() << "Cannot find string indexed " << sym.st_name << ", skipped" << endl;
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