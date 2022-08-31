# elf-on-windows

It can run native unmodified Linux binary file on Windows.

There are many people working on this,
you can search "line is not emulator" to get more information.

It can work in some cases, but it is still much work to do.

In a long time, I will not working on this.

## Build and Use

It builds by [Msys2](https://www.msys2.org/) and [xmake](https://xmake.io/).
It uses features in c++17.

[hexyl](https://github.com/sharkdp/hexyl) is used to dump binary to hex to inline in c++ code.
It should be add in `$PATH`.

```bash
git clone github.com/byronwanbl/elf-on-windows
cd elf-on-windows
xmake config --toolchain=msys2
xmake

# Dump `call_wrapper_linux_to_windows.S` to inlined hex
python3 ./asm_inline_hex.py

# Copy .exe file and .dll files
cp build/msys/x86_64/release/elf-on-windows.exe ./
cp /usr/bin/msys-2.0.dll ./
cp /usr/bin/msys-gcc_s-seh-1.dll ./
cp /usr/bin/msys-stdc++-6.dll ./

./elf-on-windows.exe test/sample1
./elf-on-windows.exe test/sample2
```

## Why

There is Bash on Windows, and it works quite well.
Why I write this?

In my opinion, Bash on Windows is too heavy.
It is unrealistic to ask users to install Bash on Windows to run a small program.
Also, Bash On Windows almost run another system, which will cost much RAM and CPU.

And it is only a experimental project for me to be familiar with something in Linux, such as elf-file format, memory management, and filesystem.

## How

It use `libc` and other libraries provided by msys.

I implemented a loader for elf-file, a dynamic linker to link with windows DLL in Msys, and a wrapper to convert different convention between Linux and windows.

Here are the structure of the project with, description:

- `src`
  - `elf_on_windows.hpp`: Main header file, defines all import classes and functions.
  - `load.cpp`: Load headers and dynamic information in elf-file.
  - `memory.cpp`: Allocate virtual memory and load segments in elf-file.
  - `dynamic_link.cpp`: Link elf-file with Windows DLL.
  - `exec.cpp`: Set up runtime environment and execute elf-file.
  - `call_wrapper.cpp` & `call_wrapper_linux_to_windows.S`: Convert convention between Linux and Windows.
  - `extra_function.cpp` & `extra_function.hpp`: Implement some nonstandard functions that does not in msys, like `__libc_start_main`.
  - `msys.cpp`: Setup msys.
- `test`
  - `sample1.c` : A simple c program.
  - `sample2.cpp` : A simple c++ program.
- `deps/include`
  - `windows_wrapper.h`: Wrap `windows.h` for code analyse.
  - `elf.h`: Structs and defines for elf-file.

## Known issues

1. Register `fs` does not support, which means stack protector and thread local variable do not work, even may cause exceptions.

2. Static linking and link to another elf-file dynamically is not support.

3. Msys use newlib, which is close to but not same with glibc. It means some nonstandard functions are nto supported.

4. It does not support command line arguments very well. You may need to modify code.

## TODOs

1. Support static linking and link to another elf-file dynamically.

2. Support register `fs`, which means to modify `FS_BASE_MSR`.

3. Support syscall.
   (It is hard to do it directly, but I think you can replace syscall to exception dynamically and handle it.)

4. Command line arguments.

5. Add more tests to make it work in more cases.

## Contribution

Issues and PRs are welcome!

## License

It licensed under LGPL version 2.1, see <https://www.gnu.org/licenses/> for more details.
