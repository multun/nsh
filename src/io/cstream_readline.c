#include "io/cstream.h"
#include "io/readline_wrapped.h"
#include "repl/repl.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/macros.h"
#include "shexp/expansion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *get_ps1(struct context *ctx)
{
    const char *res = environment_var_get(ctx->env, "PS1");
    if (res == NULL)
        return "nsh> ";
    return res;
}

static const char *get_ps2(struct context *ctx)
{
    const char *res = environment_var_get(ctx->env, "PS2");
    if (res == NULL)
        return "> ";
    return res;
}

static char *prompt_get(struct cstream *cs)
{
    struct context *ctx = cs->context;
    const char *res = (ctx->line_start ? get_ps1 : get_ps2)(ctx);
    ctx->line_start = false;

    struct ex_context ex_context;
    struct ex_scope ex_scope = EXCEPTION_SCOPE(&ex_context, NULL);

    if (setjmp(ex_scope.env)) {
        /* if an error occurs, use the unexpanded prompt */
        return strdup(res);
    }

    return expand_nosplit(&cs->line_info, res, EXP_FLAGS_PROMPT, ctx->env, &ex_scope);
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

    int res = str[cs->line_position];
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
