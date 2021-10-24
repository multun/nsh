#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>

static void parse_pattern(struct wordlist *res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    while (true) {
        wordlist_push(res, parse_word(lexer, ex_scope));
        const struct token *tok = lexer_peek(lexer, ex_scope);
        if (!tok_is(tok, TOK_PIPE))
            break;
        lexer_discard(lexer, ex_scope);
    }
}

static void parse_case_item(struct case_item_vect *vect, struct lexer *lexer, struct ex_scope *ex_scope)
{
    while (true) {
        parse_newlines(lexer, ex_scope);
        const struct token *tok = lexer_peek(lexer, ex_scope);
        // stop if esac
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse a new case item
        struct shast_case_item *case_item = zalloc(sizeof(*case_item));
        shast_case_item_init(case_item);
        case_item_vect_push(vect, case_item);
        parser_consume_optional(lexer, TOK_LPAR, ex_scope);
        parse_pattern(&case_item->pattern, lexer, ex_scope);
        parser_consume(lexer, TOK_RPAR, ex_scope);

        // skip newlines
        parse_newlines(lexer, ex_scope);
        tok = lexer_peek(lexer, ex_scope);
        // continue if there's no body
        if (tok_is(tok, TOK_DSEMI)) {
            lexer_discard(lexer, ex_scope);
            continue;
        }

        // stop if the case ends here
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse the case body
        parse_compound_list(&case_item->action, lexer, ex_scope);
        parser_consume_optional(lexer, TOK_DSEMI, ex_scope);
    }
}

void parse_rule_case(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    lexer_discard(lexer, ex_scope);
    struct shword *case_name = parse_word(lexer, ex_scope);
    struct shast_case *case_node = shast_case_attach(res, lexer);
    case_node->var = case_name;
    parse_newlines(lexer, ex_scope);
    parser_consume(lexer, TOK_IN, ex_scope);
    parse_newlines(lexer, ex_scope);
    parse_case_item(&case_node->cases, lexer, ex_scope);
    lexer_discard(lexer, ex_scope);
}
