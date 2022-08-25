#ifndef ELF_ON_WINDOWS__debug_H
#define ELF_ON_WINDOWS__debug_H

#include <cstdint>
#include <iostream>

namespace elf_on_windows
{
namespace _debug
{
struct Hex
{
    uint64_t x;
    friend std::ostream& operator<<(std::ostream& out, Hex x)
    {
        return out << "0x" << std::hex << x.x << std::oct;
    }
};
struct Range
{
    uint64_t from, size;
    friend std::ostream& operator<<(std::ostream& out, Range x)
    {
        return out << Hex { x.from } << " ~ " << Hex { x.from + x.size }
                   << " (size = " << Hex { x.size } << ")";
    }
};
}

std::ostream& debug() { return std::cerr << "[DEBUG] "; }
std::ostream& info() { return std::cerr << "[INFO] "; }
std::ostream& warning() { return std::cerr << "[WARNING] "; }
std::ostream& error() { return std::cerr << "[ERROR] "; }
using std::cerr;
using std::endl;

_debug::Hex hex(uint64_t x) { return _debug::Hex { x }; };
_debug::Range range(uint64_t from, uint64_t size) { return _debug::Range { from, size }; }
}

#endif