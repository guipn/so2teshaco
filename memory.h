/*
 * Shell - Sistemas Operacionais II
 *
 * Prof. F. J. Monaco
 * PAE:  Rene S. P.
 *
 * Guilherme P. Nami
 * 4450128
 *
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

void *_malloc(size_t);
void *_realloc(void *, size_t);
void release_resources(void);

#endif

