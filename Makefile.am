ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = 42sh

SUBDIRS = src/shlex

EXTRA_DIST = $(srcdir)/tests

check-local:
	$(srcdir)/tests/check.sh


42sh_LDADD = src/shlex/libshlex.la
42sh_SOURCES = src/main.c
