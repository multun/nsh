#pragma once


/**
** \brief what execution mode was expressed using the cli?
** \desc a file with zero arguments read from stdin
*/
enum shsrc
{
  SHSRC_FILE,
  SHSRC_COMMAND,
};


/**
** \brief all command line options
*/
struct cmdopts
{
  // was the norc option passed?
  int norc;

  // the source to read data from.
  // the underlying type is enum shsrc, but it's an
  // int to comply with getopt
  int src;
};


extern struct cmdopts g_cmdopts;


#define CMDOPTS_STATUS(Code) (-((Code) + 1))

/**
** \brief parse command line options
** \return if negative, the parsing failed and the expected
**   return code is -(retcode + 1). otherwise, the index of
**   the first non-option argument is returned
*/
int cmdopts_parse(int argc, char *argv[]);
