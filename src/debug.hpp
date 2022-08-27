#ifndef ELF_ON_WINDOWS_DEBUG_HPP
#define ELF_ON_WINDOWS_DEBUG_HPP

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

static std::ostream& debug() { return std::cerr << "[DEBUG]   "; }
static std::ostream& info() { return std::cerr << "[INFO]    "; }
static std::ostream& warning() { return std::cerr << "[WARNING] "; }
static std::ostream& error() { return std::cerr << "[ERROR]   "; }
using std::cerr;
using std::endl;

static _debug::Hex hex(uint64_t x) { return _debug::Hex { x }; };
static _debug::Range range(uint64_t from, uint64_t size) { return _debug::Range { from, size }; }
}

#endif