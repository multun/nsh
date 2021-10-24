lexer_sources = [
    'lexer.c',
    'lexer_error.c',
    'print.c',
    'sublexers.c',
    'sublexer_regular.c',
    'tok.c',
    'wlexer.c',
]

lexer_api = include_directories('api', is_system: true)
lexer_lib = library(
    'lexer',
    lexer_sources,
    install: true,
    include_directories: lexer_api,
    dependencies: [nsh_utils, nsh_io],
)

nsh_lexer = declare_dependency(
    include_directories: lexer_api,
    link_with: lexer_lib,
)