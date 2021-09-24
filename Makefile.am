PYTHON3 ?= python3

include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = nsh

SUBDIRS = \
    src/nsh \
    src/io \
    src/shexec \
    src/shlex \
    src/shparse \
    src/shwlex \
    src/utils


include $(top_srcdir)/tests/itests.mk
EXTRA_DIST = $(srcdir)/tests/check.py $(itest_files)


check-local:
	$(PYTHON3) $(srcdir)/tests/check.py $(top_builddir)/nsh

doc:
	doxygen $(srcdir)/doc/doxyconfig


# the order of this list matters
# each lib must have all of its dependencies below it
nsh_LDADD = \
    src/nsh/libnsh.la \
    src/shexec/libshexec.la \
    src/shparse/libparse.la \
    src/shlex/libshlex.la \
    src/shwlex/libshwlex.la \
    src/io/libio.la \
    src/utils/libutils.la

# there are no sources, as everything is contained within libnsh
nsh_SOURCES =

.PHONY: doc
