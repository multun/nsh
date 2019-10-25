#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

static void parse_pattern(struct wordlist *res, struct lexer *lexer, struct errcont *errcont)
{
    while (true) {
        wordlist_push(res, parse_word(lexer, errcont));
        const struct token *tok = lexer_peek(lexer, errcont);
        if (!tok_is(tok, TOK_PIPE))
            break;
        lexer_discard(lexer, errcont);
    }
}

static void parse_case_item(struct case_item_vect *vect, struct lexer *lexer, struct errcont *errcont)
{
    while (true) {
        parse_newlines(lexer, errcont);
        const struct token *tok = lexer_peek(lexer, errcont);
        // stop if esac
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse a new case item
        struct shast_case_item *case_item = zalloc(sizeof(*case_item));
        shast_case_item_init(case_item);
        case_item_vect_push(vect, case_item);
        parser_consume_optional(lexer, TOK_LPAR, errcont);
        parse_pattern(&case_item->pattern, lexer, errcont);
        parser_consume(lexer, TOK_RPAR, errcont);

        // skip newlines
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        // continue if there's no body
        if (tok_is(tok, TOK_DSEMI)) {
            lexer_discard(lexer, errcont);
            continue;
        }

        // stop if the case ends here
        if (tok_is(tok, TOK_ESAC))
            break;

        // parse the case body
        parse_compound_list(&case_item->action, lexer, errcont);
        parser_consume_optional(lexer, TOK_DSEMI, errcont);
    }
}

void parse_rule_case(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    lexer_discard(lexer, errcont);
    char *case_name = parse_word(lexer, errcont);
    struct shast_case *case_node = shast_case_attach(res, lexer);
    case_node->var = case_name;
    parse_newlines(lexer, errcont);
    parser_consume(lexer, TOK_IN, errcont);
    parse_newlines(lexer, errcont);
    parse_case_item(&case_node->cases, lexer, errcont);
    lexer_discard(lexer, errcont);
}
