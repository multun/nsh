#pragma once

#include <nsh_utils/macros.h>
#include <stdbool.h>
#include <stdio.h>


enum nsh_loglevel
{
    NSH_LOG_TRACE = 0,
    NSH_LOG_DEBUG,
    NSH_LOG_INFO,
    NSH_LOG_WARN,
    NSH_LOG_ERROR,
    NSH_LOG_DISABLED,
};

#ifndef NSH_LOG_DOMAIN
#define NSH_LOG_DOMAIN_FMT NULL
#else
#define NSH_LOG_DOMAIN_FMT XSTRINGIFY(NSH_LOG_DOMAIN)
#endif


#ifndef NDEBUG
#define nsh_trace(...) nsh_log(NSH_LOG_TRACE, NSH_LOG_DOMAIN_FMT, __FILE__, __LINE__, __VA_ARGS__)
#define nsh_debug(...) nsh_log(NSH_LOG_DEBUG, NSH_LOG_DOMAIN_FMT, __FILE__, __LINE__, __VA_ARGS__)
#define nsh_info(...)  nsh_log(NSH_LOG_INFO,  NSH_LOG_DOMAIN_FMT, __FILE__, __LINE__, __VA_ARGS__)
#define nsh_warn(...)  nsh_log(NSH_LOG_WARN,  NSH_LOG_DOMAIN_FMT, __FILE__, __LINE__, __VA_ARGS__)
#define nsh_error(...) nsh_log(NSH_LOG_ERROR, NSH_LOG_DOMAIN_FMT, __FILE__, __LINE__, __VA_ARGS__)
#else  // NDEBUG
#define nsh_trace(...)
#define nsh_debug(...)
#define nsh_info(...)
#define nsh_warn(...)
#define nsh_error(...)
#endif // NDEBUG


void nsh_log(enum nsh_loglevel level, const char *domain, const char *file, int line, const char *fmt, ...);


/**
** \brief Configures logging
** \param fp the log sink
** \param setting Logging settings are given in the following format:
**                "domain_a:INFO,domain_c,domain_b:DEBUG,*:WARN"
**                the default logging level is INFO
** \param enable_colors whether to output ANSI color codes
*/
void nsh_log_setup(FILE *fp, const char *settings, bool enable_colors);

/** Releases allocated resources */
void nsh_log_teardown();

/** Sets up loading by inspecting the LOGFILE and DEBUG environment variables */
void nsh_log_setup_environ();
