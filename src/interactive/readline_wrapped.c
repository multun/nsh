#include "readline_wrapped.h"
#include <nsh_io/keyboard_interrupt.h>
#include <nsh_utils/attr.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>

// readline's header requires including stdio beforehand
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

/** When the user presses CTRL + C, this flag is set by the signal handler */
static volatile bool interrupted;

/** The newly completed line (NULL encodes CTRL + D) */
static char *user_input;
static bool user_input_complete;

/** This is called by readline when it has read an entire line */
static void readline_callback(char *line)
{
    user_input = line;
    user_input_complete = true;
    rl_callback_handler_remove();
}


/** Checks whether user input was interupted */
static nsh_err_t check_interrupt(struct environment *env);


nsh_err_t readline_wrapped(char **res, struct environment *env, char *prompt)
{
    nsh_err_t err;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fileno(stdin), &rfds);

    // reset the interupt flag
    interrupted = false;
    rl_callback_handler_install(prompt, readline_callback);
    free(prompt);
    while (true) {
        // wait for data to be available on stdin. if the user presses CTRL + C,
        // select will instead fail and errno be set to EINTR
        if (select(1, &rfds, NULL, NULL, NULL) < 0) {
            if (errno == EAGAIN)
                continue;

            if (errno == EINTR) {
                if ((err = check_interrupt(env)))
                    return err;
                continue;
            }

            warnx("select failed in readline loop");
            return NSH_IO_ERROR;
        }

        // tell readline a character is available on stdin
        rl_callback_read_char();

        if (user_input_complete) {
            user_input_complete = false;
            *res = user_input;
            return NSH_OK;
        }

        // check again if the user pressed CTRL + C
        if ((err = check_interrupt(env)))
            return err;
    }
}


static void sigint_handler(int signal __unused)
{
    interrupted = true;
}

void readline_wrapped_setup(void)
{
    signal(SIGINT, sigint_handler);
}

static nsh_err_t check_interrupt(struct environment *env)
{
    if (!interrupted)
        return NSH_OK;

    interrupted = false;

    // reset the prompt
    rl_free_line_state();
    rl_cleanup_after_signal();
    RL_UNSETSTATE(RL_STATE_ISEARCH | RL_STATE_NSEARCH | RL_STATE_VIMOTION
                  | RL_STATE_NUMERICARG | RL_STATE_MULTIKEY);
    rl_done = 1;
    rl_callback_handler_remove();

    // let the user know what happened
    fputs("^C\n", stderr);

    // return an error
    env->code = 128 + SIGINT;
    return NSH_KEYBOARD_INTERUPT;
}
