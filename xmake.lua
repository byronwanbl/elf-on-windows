add_rules("mode.debug")

toolchain("msys2")
    set_kind("standalone")
    set_sdkdir("D:\\bin\\msys2\\usr")

target("elf-on-windows")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("deps/include", "src")
