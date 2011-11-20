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

#include "jobs.h"
#include "memory.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>


extern char       **path; /* main.c */
extern job_node_t *jobs;  /* main.c */

/**
 * $name error;
 * $proto static void error(char *msg);
 *
 * Exibe a mensagem de erro dada e chama exit(EXIT_FAILURE).
 */

static void error(char *msg)
{
    fprintf(stderr, msg);
    exit(EXIT_FAILURE);
}


/**
 * $name _malloc;
 * $proto void *_malloc(size_t size);
 *
 * Automatiza a alocacao com checagem de erro. Caso malloc retorne
 * NULL, mostra uma mensagem de erro e aborta a execucao.
 */

void *_malloc(size_t size)
{
    void *ret = malloc(size);
    
    if (!ret)
    {
	error("Erro de alocacao de memoria. Abortando execucao.");
    }

    return ret;
}


/**
 * $name _realloc;
 * $proto void *_realloc(void *orig, size_t new_size);
 *
 * Implementa comportamento analogo ao de _malloc.
 */

void *_realloc(void *orig, size_t new_size)
{
    void *ret = realloc(orig, new_size);
    
    if (!ret)
    {
	error("Erro de realocacao de memoria. Abortando execucao.");
    }

    return ret;
}


/**
 * $name release_resources
 * $proto void release_resources(void);
 *
 * Libera recursos persistentes alocados pelo shell durante sua
 * execucao.
 */

void release_resources(void)
{
    destroy_job_list(jobs);

    if (path)
        free_strings(path);
}

