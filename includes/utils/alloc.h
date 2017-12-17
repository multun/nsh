#pragma once

#include <stddef.h>

/**
** \brief safe malloc wrapper
** \param size the size to allocate
** \return a pointer to the allocated memory
*/
void *xmalloc(size_t size);
/**
** \brief a safe realloc wrapper
** \param ptr the pointer to reallocate
** \param size the new size to allocate
** \return a pointer to the allocated memory
*/
void *xrealloc(void *ptr, size_t size);
/**
** \brief safe calloc wrapper
** \param nmemb number of element to allocate
** \param size the size of one element to allocate
** \return a pointer to the allocated memory
*/
void *xcalloc(size_t nmemb, size_t size);
