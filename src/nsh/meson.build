nsh_sources = [
    'nsh.c',
    'cstream_readline.c',
    'managed_stream.c',
    'builtin_history.c',
    'readline_wrapped.c',
    'cmdopts.c',
]

cc = meson.get_compiler('c')
readline_dep = cc.find_library('readline', required: true)

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration : conf_data)

nsh = executable(
    'nsh',
    nsh_sources,
    install: true,
    link_with: [nsh_exec, nsh_io, nsh_utils],
    include_directories: nsh_headers,
    dependencies: [readline_dep],
)
