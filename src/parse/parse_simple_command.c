#include <assert.h>
#include <string.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>

#include "parse.h"

static int parse_assignment(struct shast_assignment **res, struct lexer *lexer)
{
    int rc;

    {
        const struct token *tok;
        if ((rc = lexer_peek(&tok, lexer)))
            return rc;

        if (!tok_is(tok, TOK_ASSIGNMENT_WORD))
            return PARSER_NOMATCH;
    }

    struct token *tok;
    if ((rc = lexer_pop(&tok, lexer)))
        return rc;

    struct shast_assignment *assign = zalloc(sizeof(*assign));
    assign->line_info = *lexer_line_info(lexer);
    char *val = strchr(tok_buf(tok), '=');
    *(val++) = '\0';
    assign->name = tok_buf(tok);
    assign->value = val;
    tok_free(tok, false);
    *res = assign;
    return NSH_OK;
}

static int parse_assignments(struct shast_block **block, struct shast ***res,
                             struct lexer *lexer)
{
    int rc;

    int parsed_assignments = 0;
    while (true) {
        /* Try to parse a redirection */
        struct shast_assignment *redir;
        if ((rc = parse_assignment(&redir, lexer)) < 0)
            return rc;

        if (rc == NSH_OK)
            parsed_assignments++;
        else if (rc == PARSER_NOMATCH)
            return parsed_assignments;

        /** Lazily create the redirection block */
        if (*block == NULL) {
            struct shast *old_root = **res;
            *block = shast_block_attach(*res, lexer);
            (*block)->command = old_root;
            *res = &(*block)->command;
        }
        assign_vect_push(&(*block)->assigns, redir);
    }
}

static int parse_argument(struct shword **res, struct lexer *lexer)
{
    int rc;
    const struct token *tok;
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;

    if (!tok_is(tok, TOK_WORD))
        return PARSER_NOMATCH;

    return parse_word(res, lexer);
}


static int parse_arguments(struct shast_cmd **cmd, struct shast **res,
                           struct lexer *lexer)
{
    int rc;

    int parsed_arguments = 0;
    while (true) {
        /* Try to parse an argument */
        struct shword *arg;
        if ((rc = parse_argument(&arg, lexer)) < 0)
            return rc;

        /* Succeed if the next token isn't an argument */
        if (rc == PARSER_NOMATCH)
            return parsed_arguments;

        parsed_arguments++;

        /** Lazily create the command */
        if (*cmd == NULL)
            *cmd = shast_cmd_attach(res, lexer);
        wordlist_push(&(*cmd)->arguments, arg);
    }
}

static int prefix_loop(struct shast ***res, struct lexer *lexer,
                       struct shast_block **block)
{
    int rc;
    bool parsed_prefix;
    do {
        parsed_prefix = false;
        if ((rc = parse_assignments(block, res, lexer)) < 0)
            return rc;
        if (rc > 0)
            parsed_prefix = true;
        if ((rc = parse_redirections(block, res, lexer)) < 0)
            return rc;
        if (rc > 0)
            parsed_prefix = true;
    } while (parsed_prefix);
    return NSH_OK;
}

/**
 * \brief Parse a simple command from the end of assignments and redirs
 * \param res where to attach the block node
 */
static int element_loop(struct shast ***res, struct lexer *lexer,
                        struct shast_block **block)
{
    int rc;
    struct shast_cmd *cmd = NULL;
    bool parsed_element;
    do {
        parsed_element = false;
        if ((rc = parse_arguments(&cmd, *res, lexer)) < 0)
            return rc;
        if (rc > 0)
            parsed_element = true;

        if ((rc = parse_redirections(block, res, lexer)) < 0)
            return rc;
        if (rc > 0)
            parsed_element = true;
    } while (parsed_element);
    return NSH_OK;
}

nsh_err_t parse_simple_command(struct shast **res, struct lexer *lexer)
{
    int rc;
    struct shast **original_root = res;
    struct shast_block *block = NULL;

    if ((rc = prefix_loop(&res, lexer, &block)))
        return rc;

    if ((rc = element_loop(&res, lexer, &block)))
        return rc;

    if (*original_root != NULL)
        return NSH_OK;

    const struct token *tok;
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;
    return parser_err(tok, "unexpected token in command: %s", TOKT_STR(tok));
}
