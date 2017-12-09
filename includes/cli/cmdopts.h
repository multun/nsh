#pragma once


enum shmode
{
  SHMODE_REGULAR,
  SHMODE_VERSION,
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
  int ast_print;
};


extern struct cmdopts g_cmdopts;
int cmdopts_parse(int argc, char *argv[]);
