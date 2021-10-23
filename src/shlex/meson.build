lexer_sources = [
    'lexer.c',
    'lexer_error.c',
    'print.c',
    'sublexers.c',
    'sublexer_regular.c',
    'tok.c',
]

nsh_lexer = library(
    'lexer',
    lexer_sources,
    install: true,
    include_directories: nsh_headers,
    link_with: [nsh_utils, nsh_io, nsh_wlexer],
)
