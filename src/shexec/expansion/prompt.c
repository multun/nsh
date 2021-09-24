#include "shexec/expansion.h"
#include "utils/parsing.h"
#include "utils/pathutils.h"

#include <err.h>
#include <limits.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


static void expand_strftime(struct expansion_state *exp_state, const char *format)
{
    char timebuf[128];

    /* Make the current time/date into a string. */
    time_t current_time;
    time(&current_time);

    struct tm *tm = localtime(&current_time);

    int written_bytes = strftime(timebuf, sizeof(timebuf), format, tm);
    if (written_bytes == 0)
        timebuf[0] = '\0';
    else
        timebuf[sizeof(timebuf) - 1] = '\0';

    expansion_push_nosplit_string(exp_state, timebuf);
}


enum wlexer_op expand_prompt_escape(struct expansion_state *exp_state, struct wlexer *wlexer, char c)
{
    /*
      TODO: implement the remaining prompt escapes
    \j     the number of jobs currently managed by the shell
    \l     the basename of the shell's terminal device name
    \s     the name of the shell, the basename of $0 (the portion following the final slash)
    \v     the version of bash (e.g., 2.00)
    \V     the release of bash, version + patch level (e.g., 2.00.0)
    \w     the current working directory, with $HOME abbreviated with a tilde (uses the value of the PROMPT_DIRTRIM variable)
    \W     the basename of the current working directory, with $HOME abbreviated with a tilde
    \!     the history number of this command
    \#     the command number of this command
     */

    /* octal escapes */
    if (c >= '0' && c <= '7') {
        assert(!wlexer_has_lookahead(wlexer));
        char res = c - '0';

        size_t max_size = 3;
        /* anything above 0o377 doesn't fit in a byte */
        if (res > 3)
            max_size = 2;

        for (size_t i = 1; i < max_size; i++) {
            int next_c = cstream_peek(wlexer->cs);
            if (next_c < '0' || next_c > '7')
                break;
            res = res * 8 + (cstream_pop(wlexer->cs) - '0');
        }

        expansion_push_nosplit(exp_state, res);
        return LEXER_OP_CONTINUE;
    }

    switch (c) {
    case 'w':
    case 'W': {
        const char *PWD = environment_var_get_cstring(expansion_state_env(exp_state), "PWD");
        if (PWD == NULL || strlen(PWD) == 0) {
            expansion_push_nosplit_string(exp_state, "<unknown PWD>");
            break;
        }

        bool replaced_tilde = false;
        const char *HOME = environment_var_get_cstring(expansion_state_env(exp_state), "HOME");
        if (HOME != NULL) {
            /* the n first characters must be the same */
            const char *trimmed_pwd = path_remove_prefix(PWD, HOME);
            if (trimmed_pwd != NULL) {
                expansion_push_nosplit_string(exp_state, "~/");
                PWD = trimmed_pwd;
                replaced_tilde = true;
            }
        }

        if (c == 'w') {
            const char *DIRTRIM = environment_var_get_cstring(expansion_state_env(exp_state), "PROMPT_DIRTRIM");
            /* ignore trimming if PROMPT_DIRTRIM is unset */
            if (DIRTRIM == NULL)
                goto push_path;

            /* parse the number of directories to keep */
            size_t dirtrim;
            if (parse_pure_integer(&dirtrim, 10, DIRTRIM) == -1)
                goto push_path;

            /* keep the dirtrim last components */
            size_t pwd_comp_count = path_count_components(PWD);
            size_t skipped_components = 0;
            if (dirtrim + /* the root component */ 1 < pwd_comp_count) {
                skipped_components = pwd_comp_count - dirtrim;
                PWD = path_skip_components(PWD, skipped_components);
            }

            /* add the elipsis */
            if (skipped_components) {
                if (replaced_tilde)
                    expansion_push_nosplit(exp_state, '/');
                expansion_push_nosplit_string(exp_state, "...");
                if (skipped_components != pwd_comp_count)
                    expansion_push_nosplit(exp_state, '/');
            }
        }

    push_path:
        expansion_push_nosplit_string(exp_state, PWD);
        break;
    }
    case 'h':
    case 'H': {
        /* get the hostname */
        char buf[HOST_NAME_MAX + 1];
        if (gethostname(buf, HOST_NAME_MAX) == -1) {
            warn("gethostname() failed");
            return LEXER_OP_FALLTHROUGH;
        }
        buf[HOST_NAME_MAX] = '\0';

        /* strip the domain if required */
        if (c == 'h') {
            char *first_dot = strchr(buf, '.');
            if (first_dot)
                *first_dot = '\0';
        }

        expansion_push_nosplit_string(exp_state, buf);
        break;
    }
    case 'n':
        expansion_push_nosplit(exp_state, '\n');
        break;
    case 'u': {
        struct passwd *pwd = getpwuid(geteuid());
        if (pwd == NULL)
            return LEXER_OP_FALLTHROUGH;

        const char *username = pwd->pw_name;
        if (username == NULL)
            return LEXER_OP_FALLTHROUGH;

        expansion_push_nosplit_string(exp_state, username);
        break;
    }
    case '$':
        expansion_push_nosplit(exp_state, geteuid() == 0 ? '#' : '$');
        break;
    case '\\':
        expansion_push_nosplit(exp_state, '\\');
        break;
    case 'a':
        expansion_push_nosplit(exp_state, '\a');
        break;
    case 'e':
        expansion_push_nosplit(exp_state, 033);
        break;
    case 'r':
        expansion_push_nosplit(exp_state, '\r');
        break;
    case '[':
        /* Start Of Heading - tells readline the next chars have a 0 printed size */
        expansion_push_nosplit(exp_state, 001);
        break;
    case ']':
        /* Start Of Text - tells readline the next chars have some printed size again */
        expansion_push_nosplit(exp_state, 002);
        break;
    case 'd':
        expand_strftime(exp_state, "%a %b %d");
        break;
    case 't':
        expand_strftime(exp_state, "%H:%M:%S");
        break;
    case 'T':
        expand_strftime(exp_state, "%I:%M:%S");
        break;
    case '@':
        expand_strftime(exp_state, "%I:%M %p");
        break;
    case 'A':
        expand_strftime(exp_state, "%H:%M");
        break;
    case 'D': {
        assert(!wlexer_has_lookahead(wlexer));
        if (cstream_peek(wlexer->cs) != '{')
            return LEXER_OP_FALLTHROUGH;
        cstream_pop(wlexer->cs);

        struct evect *res = &exp_state->result.string;
        int initial_size = evect_size(res);
        while (true) {
            int next_c = cstream_peek(wlexer->cs);
            if (next_c == EOF)
                expansion_error(exp_state, "unexpected EOF in \\D{format} prompt escape");
            if (next_c == '}') {
                cstream_pop(wlexer->cs);
                break;
            }
            evect_push(res, cstream_pop(wlexer->cs));
        }

        evect_push(res, '\0');
        evect_cut(res, initial_size);
        expand_strftime(exp_state, evect_data(res));
        break;
    }

    default:
        return LEXER_OP_FALLTHROUGH;
    }

    return LEXER_OP_CONTINUE;
}
