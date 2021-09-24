include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = librepl.la

librepl_la_SOURCES = \
    $(TOP_INCL)/repl/history.h \
    $(TOP_INCL)/repl/repl.h \
    context.c \
    history.c \
    repl.c \
    cmdopts.c

librepl_la_LIBADD = -lreadline
