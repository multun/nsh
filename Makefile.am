include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = 42sh

SUBDIRS = src/shlex src/io src/shparse src/utils src/shexec \
          src/cli src/repl


include $(top_srcdir)/tests/itests.mk
EXTRA_DIST = $(srcdir)/tests/check.py $(itest_files)


check-local:
	$(srcdir)/tests/check.py $(top_builddir)/42sh

doc:
	doxygen $(srcdir)/doc/doxyconfig


42sh_LDADD = src/repl/librepl.la src/shparse/libparse.la src/shexec/libshexec.la \
             src/cli/libcli.la src/shlex/libshlex.la src/io/libio.la \
             src/utils/libutils.la

42sh_SOURCES = src/main.c

.PHONY: doc
