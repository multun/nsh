AC_PREREQ([2.64])
AC_CONFIG_MACRO_DIR([m4])
AC_INIT([nsh], [1.0], [], [nsh], [https://github.com/multun/nsh/])

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT(disable-shared)
AC_PROG_CC_C99

# check for libraries
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([readline.h])
AC_CHECK_HEADERS([readline/history.h])

AC_CONFIG_HEADERS([includes/gen/config.h])
AC_CONFIG_FILES([Makefile src/shlex/Makefile src/io/Makefile
                 src/shparse/Makefile src/utils/Makefile
                 src/shexec/Makefile
                 src/repl/Makefile src/shexp/Makefile
                 src/shwlex/Makefile])

AC_OUTPUT
