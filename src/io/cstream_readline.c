#include "io/cstream.h"
#include "io/readline_wrapped.h"
#include "repl/repl.h"
#include "shexec/variable.h"
#include "utils/alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *cont_get_var(s_context *cont, const char *vname, const char *def)
{
    struct pair *hpair = htable_access(cont->env->vars, vname);
    if (!hpair)
        return def;

    // TODO: check weird sequence var->value ... if var
    s_var *var = hpair->value;
    if (!var->value)
        return def;
    return var ? var->value : "";
}

static const char *get_ps1(s_context *context)
{
    return cont_get_var(context, "PS1", "42sh> ");
}

static const char *get_ps2(s_context *context)
{
    return cont_get_var(context, "PS2", "> ");
}

static const char *prompt_get(struct cstream *cs)
{
    const char *res = (cs->context->line_start ? get_ps1 : get_ps2)(cs->context);
    cs->context->line_start = false;
    return res;
}

static int readline_io_reader_unwrapped(struct cstream_readline *cs)
{
    char *str = cs->current_line;

    if (!str) {
        const char *prompt = prompt_get(&cs->base);
        str = cs->current_line = readline_wrapped(prompt);
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

struct io_backend io_readline_backend = {
    .reader = readline_io_reader,
    .dest = readline_io_dest,
};

void cstream_readline_init(struct cstream_readline *cs)
{
    cstream_init(&cs->base, &io_readline_backend, true);
    cs->base.line_info = LINEINFO("<tty>", NULL);
    readline_wrapped_setup();
}
