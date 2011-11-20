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

#ifndef IO_H
#define IO_H

int redirect_io(char **);
void handle_pipe(int, int, int *);
void ensure_open(int);
void ensure_close(int);
void ensure_dup2(int);

#endif

