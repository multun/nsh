#pragma once

#include "utils/error.h"
#include "wordlist.h"


/**
** \brief represents an assignment node
*/
typedef struct aassignment
{
  s_wordlist *name;
  s_wordlist *value;
  struct ast *action;
} s_aassignment;


#define AASSIGNMENT(Name, Value, Action)                                     \
  ((s_aassignment)                                                           \
  {                                                                           \
    .name = Name,                                                             \
    .value = Value,                                                           \
    .action = Action,                                                         \
  })

#define AST_AASSIGNMENT(Name, Value, Action)                                 \
  AST(SHNODE_ASSIGNMENT, assignment, AASSIGNMENT(Name, Value, Action))


/**
** \brief print in dot format the representation of an assignment node
*/
void assignment_print(FILE *f, struct ast *ast);


/**
** \brief make an assignment
*/
void assign_var(s_env *env, char *name, char *value);


/**
** \brief exec an assignment node
*/
int assignment_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free an assignment node
*/
void assignment_free(struct ast *ast);
