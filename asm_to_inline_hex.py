#!/usr/bin/python

import subprocess
import re


src = "src/call_wrapper_linux_to_windows.S"
target = "src/call_wrapper_linux_to_windows.hex"

subprocess.run(f"as {src} -o /tmp/a.out", check=True, shell=True)

proc = subprocess.run(f"readelf -a /tmp/a.out",
                      capture_output=True, check=True, shell=True, text=True)

info = re.search(r".text\s+\S+\s+\S+\s+(\S+)\s+(\S+)",
                 proc.stdout, re.MULTILINE)
from_ = int(info.group(1), 16)
size = int(info.group(2), 16)

print("from_ =", hex(from_), "size =", hex(size))

proc = subprocess.run(
    f"hexyl /tmp/a.out -p -s {from_} -l {size}", capture_output=True, check=True, shell=True, text=True)

text = map(lambda s: "0x"+s, proc.stdout.replace("\n", " ").split(" "))
text = ("0x" + s.strip()
        for s in proc.stdout.replace("\n", " ").split(" ") if len(s.strip()) != 0)

with open(target, "w") as f:
    f.write("// Auto generated. DO NOT EDIT!\n")
    f.write(", ".join(text))
