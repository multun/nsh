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

nsh = executable(
    'nsh',
    nsh_sources,
    c_args: ['-DVERSION="@0@"'.format(meson.project_version())],
    install: true,
    link_with: [nsh_exec, nsh_io, nsh_utils],
    include_directories: nsh_headers,
    dependencies: [readline_dep],
)
