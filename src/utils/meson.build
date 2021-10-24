utils_sources = [
    'exception.c',
    'evect.c',
    'hash_table.c',
    'lineinfo.c',
    'mprintf.c',
    'parsing.c',
    'pathutils.c',
    'pvect.c',
    'refcnt.c',
    'signal_lut.c',
    'signal_manager.c',
    'signal_pipe.c',
    'xalloc.c',
]

utils_api = include_directories('api', is_system: true)
utils_lib = library(
    'nsh_utils',
    utils_sources,
    install: true,
    include_directories: utils_api,
)

nsh_utils = declare_dependency(
    link_with: utils_lib,
    include_directories: utils_api,
)
