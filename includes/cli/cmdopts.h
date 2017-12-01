#pragma once


enum shmode
{
  SHMODE_REGULAR,
  SHMODE_VERSION,
  SHMODE_AST_PRINT,
  SHMODE_TOKEN_PRINT,
};


enum shsrc
{
  SHSRC_FILE,
  SHSRC_COMMAND,
};


struct cmdopts
{
  int norc;
  int src;
  int shmode;
};


extern struct cmdopts g_cmdopts;
int cmdopts_parse(int argc, char *argv[]);
