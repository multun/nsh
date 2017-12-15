include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = libshexec.la

libshexec_la_SOURCES = ast_print.c test.c assignment.c case.c function.c  \
                       cmd.c if.c until.c bool_op.c for.c list.c while.c  \
                       redirection.c wordlist.c ast_exec.c environment.c  \
                       pipe.c block.c redirection_free.c ast_list.c \
                       $(TOP_INCL)/shexec/args.h args.c \
                       $(TOP_INCL)/shexec/environment.h \
                       $(TOP_INCL)/shexec/builtins.h \
                       $(TOP_INCL)/shexec/clean_exit.h clean_exit.c \
                       builtins/shopt.c builtins/echo.c builtins/cd.c \
                       builtins/exit.c builtins.c redirection_utils.c \
                       builtins/break.c break.c builtins/export.c \
                       $(TOP_INCL)/shexec/redirection_utils.h \
                       $(TOP_INCL)/shexec/break.h \
                       $(TOP_INCL)/shexp/variable.h \
                       builtins/source.c builtins/history.c \
                       subshell.c
