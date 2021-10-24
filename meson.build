project(
    'nsh',
    'c',
    version: '0.1',
    license: 'MIT',
)

cflags = ['-D_GNU_SOURCE', '-D_POSIX_C_SOURCE=200809L']
add_project_arguments(cflags, language: 'c')

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
    dependencies: [
        nsh_io,
        nsh_interactive,
        nsh_exec,
    ],
)

if get_option('doc')
	subdir('doc')
endif
