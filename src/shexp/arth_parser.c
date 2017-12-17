#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


static s_arth_ast *arth_parse_pow(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "**", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_POW, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_div(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "/", false);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_DIV, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_time(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "*", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_TIME, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_minus(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "-", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));

  if (pos == start)
    *ast = ARTH_AST(ARTH_MINUS, arth_parse_rec(start + 1, end, err), NULL);
  else
    *ast = ARTH_AST(ARTH_MINUS, arth_parse_rec(start, pos, err),
                    arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_plus(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "+", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;

  if (pos == start)
    return arth_parse_rec(start + 1, end, err);

  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_PLUS, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_band(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "&", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_BAND, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_xor(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "^", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_XOR, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_bor(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "|", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_BOR, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_and(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "&&", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_AND, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *arth_parse_or(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "||", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_OR, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


static s_arth_ast *(*arth_parser_utils[])(char **start, char **end, bool *err) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_PARSER_UTILS)
};


s_arth_ast *arth_parse_rec(char **start, char **end, bool *err)
{
  if (*err || start == end)
  {
    *err = true;
    return NULL;
  }
  s_arth_ast *res = NULL;

  for (int i = 0; i < 13; i++)
    if ((res = arth_parser_utils[i](start, end, err)))
      return res;
  *err = true;
  return NULL;
}


s_arth_ast *arth_parse(char *str, bool *err)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = arth_parse_rec(elms, end, err);
  return res;
}
