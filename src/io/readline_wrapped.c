#include "io/readline_wrapped.h"
#include "utils/attr.h"

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>

// readline's header requires including stdio beforehand
#include <readline/history.h>
#include <readline/readline.h>

/*
** This mess must be the worst part of the whole project.
** Here's what the following code does:
**  - Set a flag when Ctrl+C occurs by catching SIGINT.
**  - Use the asynchronous readline API, wait for key presses using select
**  - call rl_callback_read_char() when needed, which will call process_line
**    when a full line was read
**  - periodically check for the flag. When present, reset readline, print ^C and redraw the prompt.
*/

static bool ctrl_c;
static void handler_sigint(int signal __unused)
{
    ctrl_c = true;
}

static bool readline_called_back;
static char *readline_result;

static void process_line(char *line)
{
    readline_called_back = true;
    readline_result = line;
    rl_callback_handler_remove();
}

static bool check_reset_prompt(const char *prompt)
{
    if (!ctrl_c)
        return false;

    ctrl_c = false;
    rl_free_line_state();
    rl_cleanup_after_signal();
    RL_UNSETSTATE(RL_STATE_ISEARCH | RL_STATE_NSEARCH | RL_STATE_VIMOTION
                  | RL_STATE_NUMERICARG | RL_STATE_MULTIKEY);
    rl_done = 1;
    rl_callback_handler_remove();
    fprintf(stderr, "^C\n");
    rl_callback_handler_install(prompt, process_line);
    return true;
}

char *readline_wrapped(const char *prompt)
{
    int rc;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fileno(stdin), &rfds);

    // reset the interupt flag
    ctrl_c = false;
    rl_callback_handler_install(prompt, process_line);
    while (true) {
        if ((rc = select(1, &rfds, NULL, NULL, NULL)) < 0) {
            if (errno == EAGAIN)
                continue;

            if (errno == EINTR) {
                check_reset_prompt(prompt);
                continue;
            }

            err(1, "select failed in readline loop");
        }

        rl_callback_read_char();
        if (readline_called_back) {
            readline_called_back = false;
            return readline_result;
        }
        check_reset_prompt(prompt);
    }
}

void readline_wrapped_setup(void)
{
    using_history();
    signal(SIGINT, handler_sigint);
}
