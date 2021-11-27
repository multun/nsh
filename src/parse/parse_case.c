#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>

#include "parse.h"


static int parse_pattern(struct wordlist *res, struct lexer *lexer)
{
    int rc;
    while (true) {
        /* Parse the pattern word */
        struct shword *word;
        if ((rc = parse_word(&word, lexer)))
            return rc;
        wordlist_push(res, word);

        /* Parse the pipe. If no pipe is found, return */
        if ((rc = parser_consume_optional(lexer, TOK_PIPE)) < 0)
            return rc;
        if (rc == PARSER_NOMATCH)
            return NSH_OK;
    }
}

static int parse_case_items(struct case_item_vect *vect, struct lexer *lexer)
{
    int rc;
    if ((rc = parse_newlines(lexer)))
        return rc;

    if ((rc = parser_consume_optional(lexer, TOK_ESAC)) != PARSER_NOMATCH)
        return rc;

    while (true) {
        struct shast_case_item *case_item = zalloc(sizeof(*case_item));
        shast_case_item_init(case_item);
        case_item_vect_push(vect, case_item);

        // ['('] pattern )
        if ((rc = parser_consume_optional(lexer, TOK_LPAR)) < 0)
            return rc;
        if ((rc = parse_pattern(&case_item->pattern, lexer)))
            return rc;
        if ((rc = parser_consume(lexer, TOK_RPAR)))
            return rc;

        // skip newlines
        if ((rc = parse_newlines(lexer)))
            return rc;

        // case foo in pattern) ;; esac
        //                      ^
        if ((rc = parser_consume_optional(lexer, TOK_DSEMI)) < 0)
            return rc;
        // if a ;; was consumed, parse the next case item
        if (rc == NSH_OK)
            continue;

        // case foo in pattern) esac
        //                      ^
        if ((rc = parser_consume_optional(lexer, TOK_ESAC)) != PARSER_NOMATCH)
            return rc;

        // parse the case body
        if ((rc = parse_compound_list(&case_item->action, lexer)))
            return rc;

        // expect either ;; or esac, or both, but at least one of these options
        if ((rc = parser_consume_optional(lexer, TOK_DSEMI)) < 0)
            return rc;

        // if a ;; was found, ignore newlines
        if (rc == NSH_OK) {
            if ((rc = parse_newlines(lexer)))
                return rc;
        }

        // case foo in pattern) echo bar;; esac
        //                                 ^
        if ((rc = parser_consume_optional(lexer, TOK_ESAC)) != PARSER_NOMATCH)
            return rc;
    }
}

int parse_case(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_CASE, lexer)))
        return rc;

    struct shword *case_name;
    if ((rc = parse_word(&case_name, lexer)))
        return rc;

    struct shast_case *case_node = shast_case_attach(res, lexer);
    case_node->var = case_name;
    if ((rc = parse_newlines(lexer)))
        return rc;
    if ((rc = parser_consume(lexer, TOK_IN)))
        return rc;
    if ((rc = parse_newlines(lexer)))
        return rc;
    return parse_case_items(&case_node->cases, lexer);
}
