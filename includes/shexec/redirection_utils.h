#pragma once

/**
** \brief makes a copy of the file detailsriptor
*/
int fd_copy(int fd);

/**
** \brief makes a copy of the file detailsriptor, and closes the original
*/
int fd_move_away(int fd);

/**
** \brief moves a file detailsriptor
*/
void fd_move(int src, int dst);
