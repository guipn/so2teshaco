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

#include "builtincmd.h"
#include "jobs.h"
#include "memory.h"
#include "str.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


extern job_node_t *jobs;

/**
 * $name my_exit;
 * $proto void my_exit(void);
 *
 * Exibe uma mensagem de despedida e termina a execucao do shell.
 */

void my_exit(void)
{
    puts("\n\tCiao!\n");
    exit(EXIT_SUCCESS);
}


/**
 * $name cd;
 * $proto int cd(char *cmd);
 *
 * Dada uma string como "cd foo", efetua a chamada
 * chdir("foo"). Se a chamada falhar, exibe a respectiva 
 * mensagem de erro setad por chdir.
 */

int cd(char *cmd)
{
    char **tokenized = split(cmd, " ");

    if (!tokenized[1])
    {
	fprintf(stderr, "\n\n Erro de sintaxe: 'cd' espera que "
		" o novo diretorio seja especificado.\n\n");
	return 1;
    }

    if (chdir(tokenized[1]) == -1)
	perror("Erro ao tentar mudar de diretorio");

    return 1;
}


void show_jobs(void)
{
    print_jobs(jobs);
}


void bg(char *cmd)
{
    char **tokenized = split(cmd, " ");

    if (!tokenized[1])
    {
	fprintf(stderr, "\n\n Erro de sintaxe: bg espera como argumento um pid.\n");
	return;
    }

    pid_t target = strtol(tokenized[1], NULL, 10);
    invoke_bg(jobs, target);
}


void fg(char *cmd)
{
    char **tokenized = split(cmd, " ");

    if (!tokenized[1])
    {
	fprintf(stderr, "\n\n Erro de sintaxe: fg espera como argumento um pid.\n");
	return;
    }

    pid_t target = strtol(tokenized[1], NULL, 10);
    invoke_fg(jobs, target);
}


/**
 * $name run_builtin;
 * $proto int run_builtin(char *cmd);
 *
 * Tenta executar um comando interno do shell (rotinas deifnidas em
 * builtincmd.c, declaradas em builtincmd.h). Retorna 1 caso o comando
 * tenha sido executado, e 0 se nao.
 */

int run_builtin(char *cmd)
{
    if (
	    strcmp(cmd, "exit") == 0 ||
	    strcmp(cmd, "quit") == 0 ||
	    strcmp(cmd, "q")    == 0
       )
    {
	my_exit();
    }
    else if (strncmp(cmd, "cd", 2) == 0)
    {
	return cd(cmd);
    }
    else if (
		strcmp(cmd, "jls")     == 0 ||
		strcmp(cmd, "joblist") == 0
	    )
    {
	show_jobs();
	return 1;
    }
    else if (
		strncmp(cmd, "bg", 2)          == 0 ||
		strncmp(cmd, "background", 10) == 0
	    )
    {
	bg(cmd);
	return 1;
    }
    else if (
		strncmp(cmd, "fg", 2)          == 0 ||
		strncmp(cmd, "foreground", 10) == 0
	    )
    {
	fg(cmd);
	return 1;
    }

    return 0;
}

