#include <nsh_io/cstream.h>
#include "readline_wrapped.h"
#include <nsh_exec/repl.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/macros.h>
#include <nsh_exec/expansion.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cstream_readline.h"


static const char default_unset_ps1[] = "nsh$ ";
static const char default_unset_ps2[] = "> ";


static char *prompt_get(struct cstream_readline *cs)
{
    struct repl *repl = cs->repl;

    const char *prompt_var_name;
    const char *default_prompt;
    if (cs->line_start) {
        prompt_var_name = "PS1";
        default_prompt = default_unset_ps1;
        cs->line_start = false;
    } else {
        prompt_var_name = "PS2";
        default_prompt = default_unset_ps2;
    }

    struct sh_string *unexpanded = environment_var_get_string(repl->env, prompt_var_name);
    if (unexpanded == NULL)
        return strdup(default_prompt);

    sh_string_get(unexpanded);

    char *res;
    if (expand_nosplit(&res, &cs->base.line_info, sh_string_data(unexpanded), repl->env,
                       EXP_FLAGS_PROMPT))
        /* if an error occurs, use the unexpanded prompt */
        res = strdup(sh_string_data(unexpanded));

    sh_string_put(unexpanded);
    return res;
}

static int readline_io_reader_unwrapped(struct cstream_readline *cs)
{
    int rc;
    char *str = cs->current_line;

    if (!str) {
        char *prompt = prompt_get(cs);
        if ((rc = readline_wrapped(&cs->current_line, cs->repl->env, prompt)))
            return rc;
        str = cs->current_line;
        cs->line_position = 0;
    }

    if (str && str[cs->line_position] == '\0') {
        free(str);
        cs->current_line = NULL;
        return '\n';
    }

    if (str == NULL)
        return CSTREAM_EOF;

    /* using an unsigned char is required to avoid sign extension */
    unsigned char res = str[cs->line_position];
    if (res == '\0')
        return CSTREAM_EOF;

    cs->line_position++;
    return res;
}

static int readline_io_reader(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    int res = readline_io_reader_unwrapped(cs);
    if (res != CSTREAM_EOF)
        evect_push(&cs->repl->line_buffer, res);
    return res;
}

static int readline_io_dest(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    free(cs->current_line);
    return NSH_OK;
}

static nsh_err_t readline_io_reset(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    free(cs->current_line);
    cs->current_line = NULL;
    cs->line_position = 0;
    cs->line_start = true;
    return NSH_OK;
}


struct io_backend io_readline_backend = {
    .reader = readline_io_reader,
    .dest = readline_io_dest,
    .reset = readline_io_reset,
};

struct cstream_readline *cstream_readline_create(struct repl *repl)
{
    struct cstream_readline *cs = zalloc(sizeof(*cs));
    cstream_init(&cs->base, &io_readline_backend, true);
    cs->repl = repl;
    cs->base.line_info = LINEINFO("<tty>", NULL);
    readline_wrapped_setup();
    cs->line_start = true;
    return cs;
}
