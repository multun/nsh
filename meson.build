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

subdir('src/utils')
subdir('src/io')
subdir('src/lex')
subdir('src/parse')
subdir('src/exec')
subdir('src/interactive')

nsh = executable(
    'nsh',
    'src/nsh.c',
    install: true,
    # TODO: try to remove the nsh_utils dep, which is indirect
    dependencies: [
        nsh_utils,
        nsh_io,
        nsh_interactive,
        nsh_exec,
        readline_dep,
    ],
)

if get_option('doc')
	subdir('doc')
endif
