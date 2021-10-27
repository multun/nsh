project(
    'nsh',
    'c',
    version: '0.1',
    license: 'MIT',
    default_options: [
        'c_std=c99',
        'warning_level=3',
        'default_library=static',
    ],
)

summary({
    'optimization': get_option('optimization'),
    'debug': get_option('debug'),
    'b_sanitize': get_option('b_sanitize'),
    'warning_level': get_option('warning_level'),
    'werror': get_option('werror'),
})

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
    c_args: ['-DNSH_LOG_DOMAIN=nsh'],
    dependencies: [
        nsh_io,
        nsh_interactive,
        nsh_exec,
        nsh_utils,
    ],
)

test_runner = meson.source_root() / 'tests/run_tests'
check = custom_target(
		'check',
		command: [test_runner, nsh] + get_option('check_args'),
        console: true,  # otherwise, the output is captured and nothing works
        # setting -Dforce_check=true forces the tests to run on built
        build_by_default: get_option('force_check'),
        # this rule doesn't actually output a file called check,
        # but it doesn't matter as the rule is always built
		output: 'check',
        build_always_stale: true,
)

if get_option('doc')
	subdir('doc')
endif
