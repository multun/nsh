#include <nsh_utils/logging.h>
#include <nsh_utils/macros.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <unistd.h>

struct log_event
{
    const char *domain;
    enum nsh_loglevel level;
    const char *file;
    int line;
};


struct domain_conf
{
    char *domain;
    enum nsh_loglevel level;
};


#define GVECT_NAME domain_conf_vec
#define GVECT_TYPE struct domain_conf
#include <nsh_utils/gvect.h>
#include <nsh_utils/gvect.defs>
#undef GVECT_NAME
#undef GVECT_TYPE


static FILE *g_logfile = NULL;
static enum nsh_loglevel g_default_loglevel = NSH_LOG_DISABLED;
static bool g_logfile_needs_fclose = false;
static void (*g_log_header)(struct log_event *ev, FILE *fp) = NULL;
static struct domain_conf_vec g_log_config;


static const char *level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR"
};


#define BRIGHT_BLUE "\x1b[94m"
#define CYAN "\x1b[36m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RED "\x1b[31m"
#define MAGENTA "\x1b[35m"
#define GRAY "\x1b[90m"
#define RESET "\x1b[0m"

static const char *level_colors[] = {
    BRIGHT_BLUE, CYAN, GREEN, YELLOW, RED, MAGENTA
};

static void log_header_simple(struct log_event *ev, FILE *fp)
{
    fprintf(
        fp, "%-7d %-5s %s:%d: ",
        getpid(), level_strings[ev->level], ev->file, ev->line);
}


static void log_header_color(struct log_event *ev, FILE *fp)
{
    fprintf(
        fp, GRAY"%-7d %s%-5s"RESET" "GRAY"%s:%d:"RESET" ",
        getpid(), level_colors[ev->level], level_strings[ev->level],
        ev->file, ev->line);
}

static bool should_log(enum nsh_loglevel level, const char *domain);

void nsh_log(enum nsh_loglevel level, const char *domain, const char *file, int line, const char *fmt, ...)
{
    if (g_logfile == NULL)
        return;

    if (!should_log(level, domain))
        return;

    struct log_event ev = {
        .domain = domain,
        .file  = file,
        .line  = line,
        .level = level,
    };

    va_list ap;

    va_start(ap, fmt);

    g_log_header(&ev, g_logfile);
    vfprintf(g_logfile, fmt, ap);
    fprintf(g_logfile, "\n");
    fflush(g_logfile);

    va_end(ap);
}


static bool should_log(enum nsh_loglevel level, const char *domain)
{
    size_t config_size = domain_conf_vec_size(&g_log_config);

    // look for the config of the logging domain
    for (size_t i = 0; i < config_size; i++) {
        struct domain_conf *domain_conf = domain_conf_vec_at(&g_log_config, i);
        if (strcmp(domain_conf->domain, domain) != 0)
            continue;
        // if the user asked to log INFO on the "network" domain,
        // we should only log message of the level INFO or more important
        return level >= domain_conf->level;
    }
    // if the domain wasn't found, don't log
    return level >= g_default_loglevel;
}


static enum nsh_loglevel parse_level(char *level)
{
    for (size_t i = 0; i < ARR_SIZE(level_strings); i++) {
        if (strcasecmp(level, level_strings[i]) == 0)
            return i;
    }
    return NSH_LOG_DISABLED;
}


static void parse_domain(const char *domain_start, const char *domain_end)
{
    enum nsh_loglevel level;
    const char *eq = strchr(domain_start, ':');
    if (eq == NULL || eq > domain_end) {
        eq = domain_end;
        level = NSH_LOG_INFO;
    } else {
        const char *level_start = eq + 1;
        char *level_str = strndup(level_start, domain_end - level_start);
        level = parse_level(level_str);
        free(level_str);
        if (level == NSH_LOG_DISABLED)
            return;
    }

    // test for the special value *
    if (strncmp(domain_start, "*", 1) == 0) {
        g_default_loglevel = level;
        return;
    }

    size_t domain_len = eq - domain_start;
    char *domain = strndup(domain_start, domain_len);
    struct domain_conf conf = {
        .domain = domain,
        .level = level,
    };
    domain_conf_vec_push(&g_log_config, conf);
}

void nsh_log_setup(FILE *fp, const char *settings, bool enable_colors)
{
    if (g_logfile != NULL) {
        warnx("tried to setup logging twice");
        return;
    }

    g_logfile = fp;
    g_log_header = enable_colors ? log_header_color : log_header_simple;
    domain_conf_vec_init(&g_log_config, 10);

    g_default_loglevel = NSH_LOG_DISABLED;
    // point to the start of the current domain
    const char *domain_start = settings;
    while (true) {
        const char *domain_end = strchr(domain_start, ',');
        if (domain_end == NULL) {
            domain_end = domain_start + strlen(domain_start);
            parse_domain(domain_start, domain_end);
            break;
        }

        parse_domain(domain_start, domain_end);
        domain_start = domain_end + 1;
    }
}

void nsh_log_teardown()
{
    if (g_logfile == NULL)
        return;
    if (g_logfile_needs_fclose)
        fclose(g_logfile);
    g_logfile = NULL;

    size_t config_size = domain_conf_vec_size(&g_log_config);
    for (size_t i = 0; i < config_size; i++) {
        struct domain_conf *domain_conf = domain_conf_vec_at(&g_log_config, i);
        free(domain_conf->domain);
    }
    domain_conf_vec_destroy(&g_log_config);
}

void nsh_log_setup_environ()
{
    const char *settings = getenv("DEBUG");
    const char *logfile = getenv("LOGFILE");

    if (settings == NULL)
        return;


    // stderr case
    if (logfile == NULL) {
        bool is_interactive = isatty(STDERR_FILENO);
        nsh_log_setup(stderr, settings, is_interactive);
        return;
    }

    // file case
    FILE *fp = fopen(logfile, "w+");
    if (fp == NULL) {
        warn("failed to initialize logger");
        return;
    }
    g_logfile_needs_fclose = true;
    nsh_log_setup(fp, settings, false);
}
