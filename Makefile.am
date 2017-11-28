include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = 42sh

SUBDIRS = src/shlex src/io src/shparse src/utils src/shexec

EXTRA_DIST = $(srcdir)/tests

check-local:
	$(srcdir)/tests/check.sh


42sh_LDADD = src/shlex/libshlex.la src/io/libio.la src/shparse/libparse.la \
             src/utils/libutils.la src/shexec/libshexec.la

42sh_SOURCES = src/main.c
