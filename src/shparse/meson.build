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

nsh_parser = library(
    'parser',
    parser_sources,
    install: true,
    include_directories: nsh_headers,
    link_with: [nsh_utils, nsh_io, nsh_lexer],
)
