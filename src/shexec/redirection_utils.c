#include "shexec/redirection_utils.h"

#include <err.h>
#include <fcntl.h>
#include <unistd.h>

int fd_copy(int fd)
{
  int copy = dup(fd);
  if (copy < 0)
    errx(1, "42sh: fd_save: Failed dup file descriptor");
  if (fcntl(copy, F_SETFD, FD_CLOEXEC) < 0)
    errx(1, "42sh: fd_save: Failed fcntl file descriptor");
  return copy;
}


int fd_move_away(int fd)
{
  int copy = fd_copy(fd);
  if (close(fd) < 0)
    errx(1, "42sh: fd_save: Failed closing file descriptor");
  return copy;
}


void fd_move(int src, int dst)
{
  if (dup2(src, dst) < 0)
    errx(1, "42sh: fd_load: Failed dup file descriptor");

  if (src != dst)
    if (close(src) < 0)
      errx(1, "42sh: fd_load: Failed closing file descriptor");
}
