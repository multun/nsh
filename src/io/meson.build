io_sources = [
    'cstream.c',
    'cstream_file.c',
    'cstream_string.c',
    'keyboard_interrupt.c',
]

cc = meson.get_compiler('c')
readline_dep = cc.find_library('readline', required: true)

nsh_io = library(
    'io',
    io_sources,
    include_directories: nsh_headers,
    link_with: nsh_utils,
    dependencies: [readline_dep],
)
