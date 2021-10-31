interactive_sources = [
    'cstream_readline.c',
    'builtin_history.c',
    'readline_wrapped.c',
    'interactive_repl.c',
    'parse_cli_options.c',
    'cstream_dispatch.c',
]

cc = meson.get_compiler('c')
readline_dep = cc.find_library('readline', required: true)

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration : conf_data)

interactive_api = include_directories('api', is_system: true)
interactive_api_includes = exec_api_includes + [interactive_api]
interactive_lib = library(
    'nsh_interactive',
    interactive_sources,
    install: true,
    c_args: ['-DNSH_LOG_DOMAIN=interactive'],
    include_directories: interactive_api,
    dependencies: [nsh_exec, nsh_utils, nsh_io, readline_dep],
)

nsh_interactive = declare_dependency(
    include_directories: interactive_api_includes,
    link_with: interactive_lib,
)
