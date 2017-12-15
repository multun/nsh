include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libshexp.la

libshexp_la_SOURCES = unquote.c expansion.c expansion_subshell.c \
                      expansion_special.c arth_lex.c arth_parser.c \
											arth_expansion.c expansion_arth.c \
											$(TOP_INCL)/arth.h \
                      $(TOP_INCL)/shexp/unquote.h \
                      $(TOP_INCL)/shexp/variable.h \
                      $(TOP_INCL)/shexp/expansion.h
