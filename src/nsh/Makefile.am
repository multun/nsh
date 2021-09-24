include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libnsh.la

libnsh_la_SOURCES = \
    nsh.c \
    cstream_readline.c \
    managed_stream.c \
    builtin_history.c \
    readline_wrapped.c \
    cmdopts.c

libnsh_la_LIBADD = -lreadline
