#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>

static void parse_pattern(struct wordlist *res, struct lexer *lexer, struct exception_catcher *catcher)
{
    while (true) {
        wordlist_push(res, parse_word(lexer, catcher));
        const struct token *tok = lexer_peek(lexer, catcher);
        if (!tok_is(tok, TOK_PIPE))
            break;
        lexer_discard(lexer, catcher);
    }
}

static void parse_case_item(struct case_item_vect *vect, struct lexer *lexer, struct exception_catcher *catcher)
{
    while (true) {
        parse_newlines(lexer, catcher);
        const struct token *tok = lexer_peek(lexer, catcher);
        // stop if esac
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse a new case item
        struct shast_case_item *case_item = zalloc(sizeof(*case_item));
        shast_case_item_init(case_item);
        case_item_vect_push(vect, case_item);
        parser_consume_optional(lexer, TOK_LPAR, catcher);
        parse_pattern(&case_item->pattern, lexer, catcher);
        parser_consume(lexer, TOK_RPAR, catcher);

        // skip newlines
        parse_newlines(lexer, catcher);
        tok = lexer_peek(lexer, catcher);
        // continue if there's no body
        if (tok_is(tok, TOK_DSEMI)) {
            lexer_discard(lexer, catcher);
            continue;
        }

        // stop if the case ends here
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse the case body
        parse_compound_list(&case_item->action, lexer, catcher);
        parser_consume_optional(lexer, TOK_DSEMI, catcher);
    }
}

void parse_rule_case(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    lexer_discard(lexer, catcher);
    struct shword *case_name = parse_word(lexer, catcher);
    struct shast_case *case_node = shast_case_attach(res, lexer);
    case_node->var = case_name;
    parse_newlines(lexer, catcher);
    parser_consume(lexer, TOK_IN, catcher);
    parse_newlines(lexer, catcher);
    parse_case_item(&case_node->cases, lexer, catcher);
    lexer_discard(lexer, catcher);
}
