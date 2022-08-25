#!/usr/bin/python
import os
import sys

key = "KEY998244353"
default_files = ("./build/mingw/x86_64/release/elf-on-windows.exe", "./test/sample")

if len(sys.argv) > 1:
    files = sys.argv[1:]
else:
    files = default_files

os.system(f"croc send --code {key} {' '.join(files)}")
