#include <stdio.h>
#include <stdlib.h>

#include "shexp/expansion_result.h"
#include "shexp/glob.h"


enum glob_type
{
    GLOB_TYPE_END = 0,
    GLOB_TYPE_REGULAR, // x
    GLOB_TYPE_WILDCARD, // ?
    GLOB_TYPE_STAR, // *
    /* globstar support isn't there yet */
    // GLOB_TYPE_GLOBSTAR, // **
    GLOB_TYPE_RANGE, // [a-bx]
};

static inline enum glob_type glob_lex(const char *pattern, const char *meta)
{
    char c = *pattern;
    assert(c != '/');
    if (c == '\0')
        return GLOB_TYPE_END;

    bool quoted = !(*meta & EXPANSION_UNQUOTED);
    if (quoted)
        return GLOB_TYPE_REGULAR;

    switch (c) {
    case '?':
        return GLOB_TYPE_WILDCARD;
    case '*':
        return GLOB_TYPE_STAR;
    case '[':
        return GLOB_TYPE_RANGE;
    default:
        return GLOB_TYPE_REGULAR;
    }
}

enum glob_class_type
{
    // glob_class_type_is_start {
    GLOB_CLASS_START = 0,
    GLOB_CLASS_NOT,
    // }
    GLOB_CLASS_END,
    GLOB_CLASS_INVALID,
    GLOB_CLASS_UNIT,
    GLOB_CLASS_RANGE,
};

static inline bool glob_class_type_is_start(enum glob_class_type type)
{
    return type == GLOB_CLASS_START || type == GLOB_CLASS_NOT;
}

static inline enum glob_class_type glob_class_lex(const char *pattern, const char *meta, enum glob_class_type prev)
{
    char c = *pattern;
    assert(c != '/');
    if (c == '\0')
        return GLOB_CLASS_INVALID;

    bool unquoted = (*meta & EXPANSION_UNQUOTED);
    if (unquoted) {
        if (c == ']' && !glob_class_type_is_start(prev))
            return GLOB_CLASS_END;

        if (c == '!' && prev == GLOB_CLASS_START)
            return GLOB_CLASS_NOT;
    }

    if (pattern[1] != '-')
        return GLOB_CLASS_UNIT;

    if (pattern[2] == '\0')
        return GLOB_CLASS_INVALID;

    if (pattern[2] == ']' && (meta[2] & EXPANSION_UNQUOTED))
        return GLOB_CLASS_UNIT;

    return GLOB_CLASS_RANGE;
}

enum glob_status glob_match(const char *pattern, const char *meta, const char *string, int flags)
{
    if (*string == '.' && (flags & GLOB_FLAGS_PERIOD) && *pattern != '.')
        return GLOB_NOMATCH;

    do {
        enum glob_type type = glob_lex(pattern, meta);
        char c = *pattern;
        switch (type) {
        case GLOB_TYPE_END:
            if (*string)
                return GLOB_NOMATCH;
            return GLOB_MATCH;
        case GLOB_TYPE_REGULAR:
            if (c != *string)
                return GLOB_NOMATCH;
            string++;
            break;
        case GLOB_TYPE_WILDCARD:
            if (*string == '\0')
                return GLOB_NOMATCH;
            string++;
            break;
        case GLOB_TYPE_STAR: {
            /* handle multiple stars as a single one (no globstar yet) */
            while (glob_lex(pattern + 1, meta + 1) == GLOB_TYPE_STAR) {
                pattern += 1;
                meta += 1;
            }

            enum glob_status rc;
            for (; *string; string++)
                if ((rc = glob_match(pattern + 1, meta + 1, string, flags & ~GLOB_FLAGS_PERIOD))
                    != GLOB_NOMATCH)
                    return rc;
            break;
        }
        case GLOB_TYPE_RANGE: {
            /* parse [a-bx] */
            bool neg = false;
            enum glob_class_type class_type = GLOB_CLASS_START;

            /* skip the [ */
            pattern++;
            meta++;
            bool matched = false;
            /* continue until ] */
            while ((class_type = glob_class_lex(pattern, meta, class_type)) != GLOB_CLASS_END)
                switch (class_type) {
                case GLOB_CLASS_START:
                case GLOB_CLASS_END:
                    abort();
                case GLOB_CLASS_INVALID:
                    return GLOB_INVALID;
                case GLOB_CLASS_NOT:
                    neg = true;
                    pattern++;
                    meta++;
                    break;
                case GLOB_CLASS_UNIT:
                    matched |= *string == *pattern;
                    pattern += 1 /* x */;
                    meta += 1;
                    break;
                case GLOB_CLASS_RANGE:
                    matched |= *string >= pattern[0] && *string <= pattern[2];
                    pattern += 3  /* a-b */;
                    meta += 3;
                    break;
                }

            /* exit if the glob doesn't match*/
            if (matched == neg)
                return GLOB_NOMATCH;

            string++;
            break;
        }
        }
        pattern++;
        meta++;
    } while (1);
}
