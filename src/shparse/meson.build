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

parser_api = include_directories('api')
parser_lib = library(
    'parser',
    parser_sources,
    install: true,
    include_directories: parser_api,
    dependencies: [nsh_lexer, nsh_utils, nsh_io],
)

nsh_parser = declare_dependency(
    include_directories: [parser_api, lexer_api],
    link_with: parser_lib,
)
