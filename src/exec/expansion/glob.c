#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <nsh_exec/expansion.h>
#include <nsh_exec/expansion_callback.h>
#include <nsh_exec/expansion_result.h>
#include <nsh_exec/glob.h>
#include <nsh_utils/alloc.h>

enum glob_tok_type
{
    GLOB_TOK_TYPE_END = 0,
    GLOB_TOK_TYPE_REGULAR, // x
    GLOB_TOK_TYPE_WILDCARD, // ?
    GLOB_TOK_TYPE_STAR, // *
    /* globstar support isn't there yet */
    // GLOB_TOK_TYPE_GLOBSTAR, // **
    GLOB_TOK_TYPE_RANGE, // [a-bx]
    GLOB_TOK_TYPE_PATHSEP, // /
};

static inline enum glob_tok_type glob_lex(struct glob_pattern *pattern, size_t i)
{
    char c = pattern->data[i];
    if (c == '\0' || c == '/')
        return GLOB_TOK_TYPE_END;

    bool quoted = !(pattern->meta[i] & EXPANSION_UNQUOTED);
    if (quoted)
        return GLOB_TOK_TYPE_REGULAR;

    switch (c) {
    case '?':
        return GLOB_TOK_TYPE_WILDCARD;
    case '*':
        return GLOB_TOK_TYPE_STAR;
    case '[':
        return GLOB_TOK_TYPE_RANGE;
    case '/':
        return GLOB_TOK_TYPE_PATHSEP;
    default:
        return GLOB_TOK_TYPE_REGULAR;
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

int glob_class_type_size(enum glob_class_type type)
{
    switch (type) {
    case GLOB_CLASS_INVALID:
        abort();
    case GLOB_CLASS_START:
    case GLOB_CLASS_NOT:
    case GLOB_CLASS_END:
    case GLOB_CLASS_UNIT:
        return 1;
    case GLOB_CLASS_RANGE:
        return 3;
    default:
        abort();
    }
}

static inline bool glob_class_type_is_start(enum glob_class_type type)
{
    return type == GLOB_CLASS_START || type == GLOB_CLASS_NOT;
}

static inline enum glob_class_type glob_class_lex(struct glob_pattern *pattern, size_t i,
                                                  enum glob_class_type prev)
{
    const char *pattern_data = &pattern->data[i];
    const char *meta_data = &pattern->meta[i];
    char c = pattern_data[0];
    if (c == '\0' || c == '/')
        return GLOB_CLASS_INVALID;

    bool unquoted = (meta_data[0] & EXPANSION_UNQUOTED);
    if (unquoted) {
        if (c == ']' && !glob_class_type_is_start(prev))
            return GLOB_CLASS_END;

        if (c == '!' && prev == GLOB_CLASS_START)
            return GLOB_CLASS_NOT;
    }


    if (pattern_data[1] != '-')
        return GLOB_CLASS_UNIT;

    if (pattern_data[2] == '\0' || pattern_data[2] == '/')
        return GLOB_CLASS_UNIT;

    if (pattern_data[2] == ']' && (meta_data[2] & EXPANSION_UNQUOTED))
        return GLOB_CLASS_UNIT;

    return GLOB_CLASS_RANGE;
}

enum glob_status glob_match(struct glob_pattern *pattern, size_t pattern_i,
                            const char *string, int flags)
{
    if (*string == '.' && (flags & GLOB_FLAGS_PERIOD) && pattern->data[pattern_i] != '.')
        return GLOB_NOMATCH;

    do {
        enum glob_tok_type type = glob_lex(pattern, pattern_i);
        char c = pattern->data[pattern_i];
        switch (type) {
        case GLOB_TOK_TYPE_PATHSEP:
            abort();
        case GLOB_TOK_TYPE_END:
            if (*string)
                return GLOB_NOMATCH;
            return GLOB_MATCH;
        case GLOB_TOK_TYPE_REGULAR:
            if (c != *string)
                return GLOB_NOMATCH;
            string++;
            break;
        case GLOB_TOK_TYPE_WILDCARD:
            if (*string == '\0')
                return GLOB_NOMATCH;
            string++;
            break;
        case GLOB_TOK_TYPE_STAR: {
            /* handle multiple stars as a single one (no globstar yet) */
            while (glob_lex(pattern, pattern_i + 1) == GLOB_TOK_TYPE_STAR)
                pattern_i++;

            enum glob_status rc;
            for (; *string; string++)
                if ((rc = glob_match(pattern, pattern_i + 1, string,
                                     flags & ~GLOB_FLAGS_PERIOD))
                    != GLOB_NOMATCH)
                    return rc;
            break;
        }
        case GLOB_TOK_TYPE_RANGE: {
            /* parse [a-bx] */
            bool neg = false;
            enum glob_class_type class_type = GLOB_CLASS_START;

            /* skip the [ */
            pattern_i++;
            bool matched = false;
            /* continue until ] */
            while ((class_type = glob_class_lex(pattern, pattern_i, class_type))
                   != GLOB_CLASS_END) {
                int class_size = glob_class_type_size(class_type);
                switch (class_type) {
                case GLOB_CLASS_START:
                case GLOB_CLASS_END:
                    abort();
                case GLOB_CLASS_INVALID:
                    return GLOB_INVALID;
                case GLOB_CLASS_NOT:
                    neg = true;
                    break;
                case GLOB_CLASS_UNIT:
                    matched |= *string == pattern->data[pattern_i];
                    break;
                case GLOB_CLASS_RANGE:
                    matched |= (*string >= pattern->data[pattern_i]
                                && *string <= pattern->data[pattern_i + 2]);
                    break;
                }
                pattern_i += class_size;
            }

            /* exit if the glob doesn't match*/
            if (matched == neg)
                return GLOB_NOMATCH;

            string++;
            break;
        }
        }
        pattern_i++;
    } while (1);
}

enum glob_type glob_parse_trivial(struct glob_pattern *pattern, size_t *pattern_i)
{
    enum glob_type glob_type = GLOB_TYPE_TRIVIAL;
    do {
        enum glob_tok_type type = glob_lex(pattern, *pattern_i);
        if (type == GLOB_TOK_TYPE_END || type == GLOB_TOK_TYPE_PATHSEP)
            break;

        if (type == GLOB_TOK_TYPE_WILDCARD || type == GLOB_TOK_TYPE_STAR)
            glob_type = GLOB_TYPE_COMPLEX;

        if (type == GLOB_TOK_TYPE_RANGE) {
            glob_type = GLOB_TYPE_COMPLEX;
            enum glob_class_type class_type = GLOB_CLASS_START;

            (*pattern_i)++;
            while ((class_type = glob_class_lex(pattern, *pattern_i, class_type))
                   != GLOB_CLASS_END) {
                if (class_type == GLOB_CLASS_INVALID)
                    return GLOB_TYPE_INVALID;

                int class_size = glob_class_type_size(class_type);
                (*pattern_i) += class_size;
            }
        }
        (*pattern_i)++;
    } while (1);

    return glob_type;
}

enum glob_type glob_parse_path(struct gpath_vect *res, struct glob_pattern *pattern)
{
    size_t pattern_i = 0;
    enum glob_type status = GLOB_TYPE_TRIVIAL;
    do {
        size_t pattern_start = pattern_i;
        enum glob_type section_status = glob_parse_trivial(pattern, &pattern_i);
        if (section_status == GLOB_TYPE_INVALID)
            return GLOB_TYPE_INVALID;

        if (section_status == GLOB_TYPE_COMPLEX)
            status = GLOB_TYPE_COMPLEX;

        size_t pattern_length = pattern_i - pattern_start;

        /* count the number of slashes */
        size_t sep_count = 0;
        while (pattern->data[pattern_i + sep_count] == '/')
            sep_count++;
        pattern_i += sep_count;

        assert(sep_count != 0 || pattern->data[pattern_i] == '\0');

        /* if there's a result vector, save the path element metadata */
        if (res != NULL) {
            struct glob_path_element *elem = zalloc(sizeof(*elem));
            elem->offset = pattern_start;
            elem->length = pattern_length;
            elem->sep_count = sep_count;
            elem->name = strndup(pattern->data + pattern_start, pattern_length);
            elem->trivial = section_status == GLOB_TYPE_TRIVIAL;
            gpath_vect_push(res, elem);
        }
    } while (pattern->data[pattern_i] != '\0');
    return status;
}

#include <sys/types.h>
#include <dirent.h>

static void path_element_end(struct glob_state *glob_state, size_t path_i)
{
    struct glob_path_element *path_elem =
        gpath_vect_get(&glob_state->path_elements, path_i);
    for (size_t i = 0; i < path_elem->sep_count; i++)
        evect_push(&glob_state->path_buffer, '/');
}

static nsh_err_t __unused_result glob_callback(struct glob_state *state,
                                               struct expansion_callback_ctx *callback)
{
    struct evect *path_buffer = &state->path_buffer;
    char *word = strdup(evect_data(path_buffer));
    return expansion_callback_ctx_call(callback, word);
}

static int glob_recurse_dir(struct glob_state *glob_state,
                            struct expansion_callback_ctx *callback,
                            struct glob_pattern *pattern, DIR *directory, size_t path_i)
{
    int rc;
    struct glob_path_element *path_elem =
        gpath_vect_get(&glob_state->path_elements, path_i);
    struct evect *path_buffer = &glob_state->path_buffer;
    bool last_path_elem = (path_i + 1 == gpath_vect_size(&glob_state->path_elements));
    bool dir_only = (last_path_elem && path_elem->sep_count > 0);

    size_t path_len = evect_size(&glob_state->path_buffer);

    size_t match_count = 0;
    struct dirent *dirent;
    while ((dirent = readdir(directory))) {
        /* non-directories can't match intermediate path elements */
        if (dirent->d_type != DT_DIR && !last_path_elem)
            continue;

        /* handle the *test/ case */
        if (dirent->d_type != DT_DIR && dir_only)
            continue;

        bool is_special =
            (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0);
        /* special directories should be skipped unless in the echo a/.* case */
        if (is_special && !last_path_elem)
            continue;

        /* skip non matches */
        if (glob_match(pattern, path_elem->offset, dirent->d_name, GLOB_FLAGS_PERIOD)
            != GLOB_MATCH)
            continue;

        /* three cases at that point:
        **  - intermediate dir
        **  - final dir
        **  - final file
        */

        evect_cut(path_buffer, path_len);
        evect_push_string(path_buffer, dirent->d_name);

        /* push the path elements terminators */
        if (dirent->d_type == DT_DIR)
            path_element_end(glob_state, path_i);

        /* either run the callback, or get one more recursion level */
        if (last_path_elem) {
            evect_push(path_buffer, '\0');
            /* callback on matches */
            if ((rc = glob_callback(glob_state, callback)) < 0)
                return rc;
            match_count++;
        } else {
            /* open and recurse */
            int subdir_fd =
                openat(dirfd(directory), dirent->d_name, O_RDONLY | O_DIRECTORY, 0);
            if (subdir_fd == -1) {
                warn("couldn't open %s", dirent->d_name);
                continue;
            }

            DIR *subdir = fdopendir(subdir_fd);
            if ((rc = glob_recurse_dir(glob_state, callback, pattern, subdir, path_i + 1))
                < 0)
                return rc;
            closedir(subdir);
        }
    }
    return match_count;
}

static size_t glob_recurse(struct glob_state *glob_state,
                           struct expansion_callback_ctx *callback,
                           struct glob_pattern *glob_pattern)
{
    size_t start_offset = 0;
    const char *base_dir = ".";
    if (gpath_vect_get(&glob_state->path_elements, 0)->length == 0) {
        start_offset = 1;
        base_dir = "/";
        path_element_end(glob_state, 0);
    }

    DIR *directory = opendir(base_dir);
    if (!directory) {
        warn("cannot open directory %s", base_dir);
        return 0;
    }

    size_t match_count =
        glob_recurse_dir(glob_state, callback, glob_pattern, directory, start_offset);
    closedir(directory);
    return match_count;
}

nsh_err_t glob_expand(struct glob_state *glob_state, struct expansion_result *result,
                      struct expansion_callback_ctx *callback)
{
    nsh_err_t err;

    struct glob_pattern pattern;
    pattern.data = expansion_result_data(result);
    pattern.meta = expansion_result_meta(result);

    /* parse a first time the glob, but don't save the path components yet */
    enum glob_type glob_type = glob_parse_path(NULL, &pattern);

    /* if the glob is invalid, or only has trivial sections, don't glob at all */
    if (expansion_result_size(result) == 0 || glob_type == GLOB_TYPE_INVALID
        || glob_type == GLOB_TYPE_TRIVIAL)
        return expansion_callback_ctx_call(callback, expansion_result_dup(result));

    glob_state_reset(glob_state);

    /* fill the path vector */
    glob_parse_path(&glob_state->path_elements, &pattern);

    if (glob_recurse(glob_state, callback, &pattern) == 0)
        // TODO: nullglob support
        if ((err = expansion_callback_ctx_call(callback, expansion_result_dup(result))))
            return err;

    return NSH_OK;
}
