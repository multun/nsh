#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "io/cstream.h"
#include "utils/error.h"
#include "utils/evect.h"


#define TOK_BUF_MIN_SIZE 10

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
    TOK_LESS,       // <
    TOK_GREAT,      // >

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

  struct evect str;
} s_token;


#define TOK_PUSH(Tok, C) evect_push(&(Tok)->str, C)
#define TOK_SIZE(Tok) (Tok)->str.size
#define TOK_STR(Tok) ((Tok)->str.data)

typedef struct lexer
{
  s_token *head;
  s_cstream *stream;
} s_lexer;

s_token *tok_alloc(void);
void tok_free(s_token *free, bool free_buf);

bool tok_is(const s_token *tok, enum token_type type);
s_token *tok_as(s_token *tok, enum token_type type);

s_lexer *lexer_create(s_cstream *stream);
void lexer_free(s_lexer *lexer);

const s_token *lexer_peek(s_lexer *lexer);
s_token *lexer_pop(s_lexer *lexer);

bool word_read(s_cstream *cs, s_token *tok, s_sherror **error);
