#pragma once


enum shsrc
{
  SHSRC_FILE,
  SHSRC_COMMAND,
};


struct cmdopts
{
  int norc;
  int src;
};


extern struct cmdopts g_cmdopts;
int cmdopts_parse(int argc, char *argv[]);
