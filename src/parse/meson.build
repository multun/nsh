parser_sources = [
    'parser_error.c',
    'parse.c',
    'parse_case.c',
    'parse_command.c',
    'parse_for.c',
    'parse_if.c',
    'parse_list.c',
    'parse_redirection.c',
    'parse_simple_command.c',
    'parse_while.c',
    'parse_word.c',
    'ast_print.c',
    'ast_free.c',
]

parse_api = include_directories('api', is_system: true)
parse_api_includes = (
    lex_api_includes +
    utils_api_includes +
    io_api_includes +
    [parse_api]
)
parse_lib = library(
    'nsh_parse',
    parser_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=parse'],
    include_directories: parse_api_includes,
    dependencies: [nsh_lex, nsh_utils, nsh_io],
)

nsh_parse = declare_dependency(
    include_directories: parse_api_includes,
    link_with: parse_lib,
)
