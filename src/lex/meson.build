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
lex_api_includes = utils_api_includes + [lex_api]
lex_lib = library(
    'nsh_lex',
    lexer_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=lex'],
    include_directories: lex_api_includes,
    dependencies: [nsh_utils, nsh_io],
)

nsh_lex = declare_dependency(
    include_directories: lex_api_includes,
    link_with: lex_lib,
)
