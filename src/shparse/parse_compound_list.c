#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static bool compound_list_end(const s_token *tok)
{
    return tok_is(tok, TOK_THEN) || tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_FI)
        || tok_is(tok, TOK_DONE) || tok_is(tok, TOK_ESAC) || tok_is(tok, TOK_ELIF)
        || tok_is(tok, TOK_DSEMI) || tok_is(tok, TOK_DO) || tok_is(tok, TOK_RBRACE)
        || tok_is(tok, TOK_RPAR);
}

static bool compound_list_loop(s_lexer *lexer, s_alist **tail, s_errcont *errcont)
{
    parse_newlines(lexer, errcont);
    const s_token *tok = lexer_peek(lexer, errcont);
    if (compound_list_end(tok))
        return true;
    (*tail)->next = xcalloc(sizeof(s_alist), 1);
    parse_and_or(&(*tail)->next->action, lexer, errcont);
    *tail = (*tail)->next;
    return false;
}

void parse_compound_list(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    parse_newlines(lexer, errcont);
    *res = ast_create(SHNODE_LIST, lexer);
    parse_and_or(&(*res)->data.ast_list.action, lexer, errcont);
    // TODO: check for exception leaks
    s_alist *tmp = &(*res)->data.ast_list;
    const s_token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND)
           || tok_is(tok, TOK_NEWLINE)) { // TODO: & = background task
        tok_free(lexer_pop(lexer, errcont), true);
        if (compound_list_loop(lexer, &tmp, errcont))
            break;
        tok = lexer_peek(lexer, errcont);
    }
}
