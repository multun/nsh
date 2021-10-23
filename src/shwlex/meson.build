wlexer_sources = [
    'wlexer.c',
]

nsh_wlexer = library(
    'wlexer',
    wlexer_sources,
    install: true,
    include_directories: nsh_headers,
    link_with: [nsh_utils, nsh_io],
)
