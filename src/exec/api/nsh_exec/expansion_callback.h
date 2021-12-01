#pragma once

#include <nsh_exec/environment.h>
#include <nsh_utils/exception.h>


typedef nsh_err_t (*expansion_callback_f)(void *data, char *word,
                                          struct environment *env);

struct expansion_callback
{
    expansion_callback_f func;
    void *data;
};

struct expansion_callback_ctx
{
    struct expansion_callback callback;
    struct environment *env;
};

static inline void expansion_callback_ctx_init(struct expansion_callback_ctx *ctx,
                                               struct expansion_callback *callback,
                                               struct environment *env)
{
    if (callback) {
        ctx->callback = *callback;
    } else {
        ctx->callback.func = NULL;
    }
    ctx->env = env;
}

static inline nsh_err_t expansion_callback_ctx_call(struct expansion_callback_ctx *ctx,
                                                    char *word) __unused_result
{
    return ctx->callback.func(ctx->callback.data, word, ctx->env);
}

static inline bool expansion_callback_ctx_available(struct expansion_callback_ctx *ctx)
{
    return ctx->callback.func != NULL;
}
