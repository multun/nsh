include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libshexec.la

libshexec_la_SOURCES = ast_print.c test.c assignment.c case.c function.c  \
                       cmd.c if.c until.c bool_op.c for.c list.c while.c  \
                       redirection.c wordlist.c ast_exec.c environment.c  \
                       pipe.c block.c redirection_free.c ast_list.c \
                       $(TOP_INCL)/shexec/environment.h cd.c \
                       $(TOP_INCL)/shexec/clean_exit.h clean_exit.c \
                       $(TOP_INCL)/shexec/expansion.h expansion.c \
                       shopt_built.c echo_built.c
