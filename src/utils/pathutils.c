#include "utils/pathutils.h"
#include "utils/cpvect.h"
#include "utils/mprintf.h"

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *home_suffix(const char *suffix)
{
    char *home = getpwuid(getuid())->pw_dir;
    char *path = malloc(strlen(home) + strlen(suffix) + 1);
    return strcat(strcpy(path, home), suffix);
}

static void path_component_scan(struct path_component *cmp)
{
    for (cmp->length = 0;; cmp->length++) {
        char c = cmp->data[cmp->length];
        if (c == '\0' || c == '/')
            break;
    }
}

void path_component_init(struct path_component *cmp, const char *data)
{
    cmp->comp_i = 0;

    /* detect the leading / for the first component */
    cmp->root = false;
    if (*data == '/') {
        cmp->root = true;
        /* remove excess '/' */
        while (data[1] == '/')
            data++;
    }
    cmp->data = data;
    /* get the length and trailing slash */
    path_component_scan(cmp);
}

void path_component_next(struct path_component *cmp)
{
    cmp->root = false;
    cmp->comp_i++;

    /* skip the content of the component */
    cmp->data += cmp->length;

    /* skip the trailing slashes */
    while (*cmp->data == '/')
        cmp->data++;

    /* get the length and trailing slash */
    path_component_scan(cmp);
}

bool path_component_valid(struct path_component *cmp)
{
    return cmp->length != 0 || cmp->root;
}

void path_component_repr(struct path_component *cmp, struct evect *dst)
{
    for (size_t i = 0; i < cmp->length; i++)
        evect_push(dst, cmp->data[i]);

    if (path_component_trailing_slash(cmp))
        evect_push(dst, '/');
}

size_t path_component_repr_size(struct path_component *cmp)
{
    size_t res = cmp->length;
    if (path_component_trailing_slash(cmp))
        res++;
    return res;
}

char *path_canonicalize(const char *complex_path)
{
    /* create an array to store the simplified path components */
    struct cpvect components;
    cpvect_init(&components, 6);

    struct path_component cmp;
    for (path_component_init(&cmp, complex_path); path_component_valid(&cmp); path_component_next(&cmp)) {
        if (path_component_eq(&cmp, "."))
            continue;

        if (path_component_eq(&cmp, "..")) {
            if (cmp.comp_i == 0)
                goto keep_component;

            const char *prev_cmp = cpvect_get(&components, cmp.comp_i - 1);
            if (cmp.comp_i == 1 && strcmp(prev_cmp, "/") == 0)
                goto keep_component;

            if (strcmp(prev_cmp, "..") == 0)
                goto keep_component;

            /* If the preceding component does not refer (in the context of pathname resolution with symbolic links followed) to a directory,
               then the cd utility shall display an appropriate error message and no further steps shall be taken. */
            free(cpvect_pop(&components));
            continue;
        }

        struct evect cur_comp;
        size_t comp_size;
    keep_component:
        /* compute the size of the component representation */
        comp_size = path_component_repr_size(&cmp);

        /* add the component representation to the path components array */
        evect_init(&cur_comp, comp_size + 1);
        path_component_repr(&cmp, &cur_comp);
        evect_finalize(&cur_comp);
        cpvect_push(&components, evect_data(&cur_comp));
    }

    /* create a buffer to contain the final simplified path */
    struct evect simple_path;
    evect_init(&simple_path, strlen(complex_path) + 1);

    /* copy all the components to the unified string */
    for (size_t i = 0; i < cpvect_size(&components); i++) {
        char *component = cpvect_get(&components, i);
        evect_push_string(&simple_path, component);
        free(component);
    }

    /* add a nul byte and remove the trailing /, if there */
    if (simple_path.size > 1 && evect_get(&simple_path, simple_path.size - 1) == '/')
        evect_data(&simple_path)[simple_path.size - 1] = '\0';
    else
        evect_finalize(&simple_path);
    return evect_data(&simple_path);
}

char *path_join(const char *base, const char *relpath)
{
    /* avoid adding a / if the base already has one */
    const char *sep = "/";
    if (base[strlen(base) - 1] == '/')
        sep = "";
    return mprintf("%s%s%s", base, sep, relpath);
}
