wlexer_sources = [
    'wlexer.c',
]


wlexer_api = include_directories('api')
wlexer_lib = library(
    'wlexer',
    wlexer_sources,
    install: true,
    include_directories: wlexer_api,
    dependencies: [nsh_utils, nsh_io],
)

nsh_wlexer = declare_dependency(
    include_directories: wlexer_api,
    link_with: wlexer_lib,
)