#pragma once

#include "shexec/environment.h"
#include "utils/error.h"


typedef void (*expansion_callback_f)(void *data, char *word, struct environment *env, struct errcont *cont);

struct expansion_callback {
    expansion_callback_f func;
    void *data;
};

struct expansion_callback_ctx {
    struct expansion_callback callback;
    struct environment *env;
    struct errcont *errcont;
};

static inline void expansion_callback_ctx_init(struct expansion_callback_ctx *ctx,
                                               struct expansion_callback *callback,
                                               struct environment *env,
                                               struct errcont *errcont)
{
    if (callback) {
        ctx->callback = *callback;
    } else {
        ctx->callback.func = NULL;
    }
    ctx->env = env;
    ctx->errcont = errcont;
}

static inline void expansion_callback_ctx_call(struct expansion_callback_ctx *ctx, char *word)
{
    ctx->callback.func(ctx->callback.data, word, ctx->env, ctx->errcont);
}

static inline bool expansion_callback_ctx_available(struct expansion_callback_ctx *ctx)
{
    return ctx->callback.func != NULL;
}
