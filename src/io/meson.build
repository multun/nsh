io_sources = [
    'cstream.c',
    'cstream_file.c',
    'cstream_string.c',
    'keyboard_interrupt.c',
]

cc = meson.get_compiler('c')
readline_dep = cc.find_library('readline', required: true)

io_api = include_directories('api', is_system: true)
io_lib = library(
    'nsh_io',
    io_sources,
    install: true,
    include_directories: io_api,
    dependencies: [nsh_utils, readline_dep],
)

nsh_io = declare_dependency(
    include_directories: io_api,
    link_with: io_lib,
)