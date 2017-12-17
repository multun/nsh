#pragma once


/**
** \brief makes a copy of the file descriptor
*/
int fd_copy(int fd);


/**
** \brief makes a copy of the file descriptor, and closes the original
*/
int fd_move_away(int fd);


/**
** \brief moves a file descriptor
*/
void fd_move(int src, int dst);
