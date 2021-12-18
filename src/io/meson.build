io_sources = [
    'cstream.c',
    'cstream_file.c',
    'cstream_string.c',
]

io_api = include_directories('api', is_system: true)
io_lib = library(
    'nsh_io',
    io_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=io'],
    include_directories: io_api,
    dependencies: [nsh_utils],
)

nsh_io = declare_dependency(
    include_directories: io_api,
    link_with: io_lib,
    dependencies: [
        nsh_utils.partial_dependency(includes: true),
    ]
)
