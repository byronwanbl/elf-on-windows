add_rules("mode.debug")

add_ldflags("-static")

target("elf-on-windows")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("deps/include")
