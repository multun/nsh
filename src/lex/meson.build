lexer_sources = [
    'lexer.c',
    'lexer_error.c',
    'print.c',
    'sublexers.c',
    'sublexer_regular.c',
    'tok.c',
    'wlexer.c',
]

lex_api = include_directories('api', is_system: true)
lex_lib = library(
    'nsh_lex',
    lexer_sources,
    install: true,
    include_directories: lex_api,
    dependencies: [nsh_utils, nsh_io],
)

nsh_lex = declare_dependency(
    include_directories: lex_api,
    link_with: lex_lib,
)