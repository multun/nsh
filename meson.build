project(
    'nsh',
    'c',
    version: '0.1',
    license: 'MIT'
)

add_global_arguments(
    ['-D_GNU_SOURCE', '-D_POSIX_C_SOURCE=200809L'],
    language: 'c'
)

nsh_headers = include_directories('includes')

subdir('src/utils')
subdir('src/io')
subdir('src/shwlex')
subdir('src/shlex')
subdir('src/shparse')
subdir('src/shexec')
subdir('src/interactive')

nsh = executable(
    'nsh',
    'src/nsh.c',
    install: true,
    link_with: [nsh_interactive, nsh_exec, nsh_io],
    include_directories: nsh_headers,
    dependencies: [readline_dep],
)
