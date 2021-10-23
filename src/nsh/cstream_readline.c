#include "io/cstream.h"
#include "readline_wrapped.h"
#include "shexec/repl.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/macros.h"
#include "shexec/expansion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *prompt_get(struct cstream *cs)
{
    struct repl *ctx = cs->context;

    const char *prompt_var_name;
    const char *default_prompt;
    if (ctx->line_start) {
        prompt_var_name = "PS1";
        default_prompt = "nsh> ";
        ctx->line_start = false;
    } else {
        prompt_var_name = "PS2";
        default_prompt = "> ";
    }

    struct sh_string *unexpanded = environment_var_get_string(ctx->env, prompt_var_name);
    if (unexpanded == NULL)
        return strdup(default_prompt);

    struct ex_context ex_context;
    struct ex_scope ex_scope = EXCEPTION_SCOPE(&ex_context, NULL);

    sh_string_get(unexpanded);

    if (setjmp(ex_scope.env)) {
        /* if an error occurs, use the unexpanded prompt */
        char *res = strdup(sh_string_data(unexpanded));
        sh_string_put(unexpanded);
        return res;
    }

    char *res = expand_nosplit(&cs->line_info, sh_string_data(unexpanded), EXP_FLAGS_PROMPT, ctx->env, &ex_scope);
    sh_string_put(unexpanded);
    return res;
}

static int readline_io_reader_unwrapped(struct cstream_readline *cs)
{
    char *str = cs->current_line;

    if (!str) {
        char *prompt = prompt_get(&cs->base);
        str = cs->current_line = readline_wrapped(cs->base.ex_scope, prompt);
        cs->line_position = 0;
    }

    if (str && str[cs->line_position] == '\0') {
        free(str);
        cs->current_line = NULL;
        return '\n';
    }

    if (str == NULL)
        return EOF;

    /* using an unsigned char is required to avoid sign extension */
    unsigned char res = str[cs->line_position];
    if (res == '\0')
        return EOF;

    cs->line_position++;
    return res;
}

static int readline_io_reader(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    int res = readline_io_reader_unwrapped(cs);
    if (res != EOF)
        evect_push(&cs->base.context->line_buffer, res);
    return res;
}

static void readline_io_dest(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    free(cs->current_line);
}

static void readline_io_reset(struct cstream *base_cs)
{
    struct cstream_readline *cs = (struct cstream_readline *)base_cs;
    free(cs->current_line);
    cs->current_line = NULL;
    cs->line_position = 0;
}


struct io_backend io_readline_backend = {
    .reader = readline_io_reader,
    .dest = readline_io_dest,
    .reset = readline_io_reset,
};

void cstream_readline_init(struct cstream_readline *cs)
{
    cstream_init(&cs->base, &io_readline_backend, true);
    cs->base.line_info = LINEINFO("<tty>", NULL);
    readline_wrapped_setup();
}
