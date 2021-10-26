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
    'runtime_error.c',
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
exec_api_includes = (
    utils_api_includes +
    io_api_includes +
    parse_api_includes +
    [exec_api]
)

exec_lib = library(
    'nsh_exec',
    exec_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=exec'],
    include_directories: exec_api_includes,
    dependencies: [nsh_utils, nsh_io, nsh_parse, nsh_lex],
)

nsh_exec = declare_dependency(
    include_directories: exec_api_includes,
    link_with: exec_lib,
)
