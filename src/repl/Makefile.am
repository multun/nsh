include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = librepl.la

librepl_la_SOURCES = context.c history.c repl.c $(TOP_INCL)/repl/repl.h \
                     $(TOP_INCL)/repl/history.h
librepl_la_LIBADD = -lreadline
