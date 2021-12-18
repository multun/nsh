exec_sources = [
    'ast_exec.c',
    'block.c',
    'bool_op.c',
    'builtins/cd.c',
    'builtins/echo.c',
    'builtins/export.c',
    'builtins/printf.c',
    'builtins/source.c',
    'builtins/unset.c',
    'cmd.c',
    'environment.c',
    'function.c',
    'managed_fork.c',
    'pipe.c',
    'redirection.c',
    'subshell.c',
    'value.c',
    'loops.c',
    'shopt.c',
    'expansion/arithmetic_expansion.c',
    'expansion/expansion.c',
    'expansion/expansion_special.c',
    'expansion/glob.c',
    'expansion/prompt.c',
    'expansion/subshell.c',
    'repl_state.c',
    'repl.c',
    'history.c',
    'clean_exit.c',
    'control_flow.c',
]

conf_data = configuration_data()
conf_data.set('MAX_CALL_DEPTH', get_option('max_call_depth'))
configure_file(output: 'config.h', configuration : conf_data)


exec_api = include_directories('api', is_system: true)
exec_lib = library(
    'nsh_exec',
    exec_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=exec'],
    include_directories: exec_api,
    dependencies: [nsh_utils, nsh_io, nsh_parse, nsh_lex],
)

nsh_exec = declare_dependency(
    include_directories: exec_api,
    link_with: exec_lib,
    dependencies: [
        nsh_parse.partial_dependency(includes: true),
    ]
)
