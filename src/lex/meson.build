lex_sources = [
    'lexer.c',
    'sublexers.c',
    'sublexer_regular.c',
    'token.c',
    'wlexer.c',
]

lex_api = include_directories('api', is_system: true)
lex_lib = library(
    'nsh_lex',
    lex_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=lex'],
    include_directories: lex_api,
    dependencies: [nsh_utils, nsh_io],
)

nsh_lex = declare_dependency(
    include_directories: lex_api,
    link_with: lex_lib,
    dependencies: [
        nsh_io.partial_dependency(includes: true),
        nsh_utils.partial_dependency(includes: true),
    ]
)
