#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "io/cstream.h"
#include "utils/dbuf.h"


typedef struct token
{
  enum token_type
  {
    TOK_WORD,
    TOK_ASSIGNEMENT_WORD,
    TOK_NAME,
    TOK_NEWLINE,
    TOK_IO_NUMBER,

    TOK_AND_IF,     // &&
    TOK_OR_IF,      // ||
    TOK_DSEMI,      // ;;
    TOK_DLESS,      // <<
    TOK_DGREAT,     // >>
    TOK_LESSAND,    // <&
    TOK_GREATAND,   // >&
    TOK_LESSGREAT,  // <>
    TOK_LESSDASH,   // <<-

    TOK_CLOBBER,    // >|

    TOK_SEMI,       // ;
    TOK_AND,        // &
    TOK_PIPE,       // |

    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_ELIF,
    TOK_FI,
    TOK_DO,
    TOK_DONE,

    TOK_CASE,
    TOK_ESAC,
    TOK_WHILE,
    TOK_UNTIL,
    TOK_FOR,

    TOK_LPAR,
    TOK_RPAR,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_BANG,
    TOK_IN,

    TOK_FUNC,

    TOK_EOF,
  } type;

  union
  {
    int io_number;
    struct dbuf str;
  } data;
} s_token;


#define TOK_STR(Tok) ((Tok)->data.str.buf)

typedef struct lexer
{
  s_token *head;
  s_cstream *stream;
} s_lexer;


bool tok_is(const s_token *tok, enum token_type type);
s_token *tok_as(s_token *tok, enum token_type type);

s_lexer *lexer_create(s_cstream *stream);
void lexer_free(s_lexer *lexer);

const s_token *lexer_peek(s_lexer *lexer);
s_token *lexer_pop(s_lexer *lexer);
