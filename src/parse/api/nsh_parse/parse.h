#pragma once

/**
** \file shparse/parse.h
*/

#include <nsh_parse/ast.h>
#include <nsh_lex/lexer.h>
#include <nsh_utils/error.h>


/**
** \brief parse the input of nsh.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
*/
nsh_err_t parse(struct shast **res, struct lexer *lexer);
