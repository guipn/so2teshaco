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

#ifndef STR_H
#define STR_H

#include <stdio.h>

void free_strings(char **);
char *read_line(FILE *);
char **split(char *, char *);
char *join(char *[], char *);
char *concat(unsigned int, char *, ...);
char **parse_PATH(void);
char **get_command_tokens(char *, int *);
size_t string_buffer_length(char **);

#endif

