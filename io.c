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

#include "io.h"
#include "str.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef debug
#include <stdio.h>
#endif

static void out_append(char *);
static void out_overwrite(char *);
static void err_append(char *);
static void err_overwrite(char *);
static void in(char *);


void ensure_open(int openret)
{
    if (openret == -1)
    {
	perror("open");
	exit(EXIT_FAILURE);
    }
}


void ensure_close(int closeret)
{
    if (closeret == -1)
    {
	perror("close");
	exit(EXIT_FAILURE);
    }
}


void ensure_dup2(int dup2ret)
{
    if (dup2ret == -1)
    {
	perror("dup2");
	exit(EXIT_FAILURE);
    }
}


/**
 * $name handle_pipe;
 * $proto void handle_pipe(int cmdindex, int lastindex, int *pipefd);
 *
 * Chama dup2 da forma necessaria dependendo do indice do comando sendo
 * processado. lastindex eh o indice do ultimo comando da lista sendo 
 * processada.
 */

void handle_pipe(int cmdindex, int lastindex, int *pipefd)
{
    if (lastindex == 0)
	return;

    if (cmdindex == 0)
    {
	puts("\n\n\tClosing fd[1], duping STDOUT_FILENO\n");
	ensure_close(close(pipefd[0]));
	ensure_dup2(dup2(pipefd[1], STDOUT_FILENO));
    }
    else 
    {
	puts("\n\n\tClosing fd[0], duping STDIN_FILENO\n");
	ensure_close(close(pipefd[1]));
	ensure_dup2(dup2(pipefd[0], STDIN_FILENO));
    }

    puts("\n\tReturning from handle_pipe.\n");
    return;
}


/**
 * $name redirect_io;
 * $proto $void redirect_io(char **tokenized);
 *
 * Se necessario, redireciona stdin/stdout/stderr para este processo.
 * Eh chamada por main.c:call_exec para configurar io.
 *
 * Retorna 0 se nao houve redirecionamento, e 1 caso contrario. Isto eh
 * necessario para que call_exec saiba se deve chamar free para os 
 * argumentos de redirecionamento, para que eles nao interfiram na substituicao
 * pelo novo processo; se isto nao for feito, os argumentos de redirecionamento
 * sao considerados como argumentos para a invocacao do novo processo.
 */

int redirect_io(char **tokenized)
{
    size_t lastpos = string_buffer_length(tokenized);

    if (lastpos >= 3)
    {
	char *iopath = tokenized[lastpos - 1];

	if (strcmp(tokenized[lastpos - 2], ">>") == 0)
	{
#ifdef debug
	    printf("\n\t(%s): Redirect stdout (append) to %s.\n", 
		    __func__, 
		    iopath);
#endif
	    out_append(iopath);
	    return 1;
	}
	else if (strcmp(tokenized[lastpos - 2], ">") == 0)
	{
#ifdef debug
	    printf("\n\t(%s): Redirect stdout (overwrite) to %s.\n",
		    __func__, 
		    iopath);
#endif
	    out_overwrite(iopath);
	    return 1;
	}
	else if (strcmp(tokenized[lastpos - 2], "2>>") == 0)
	{
#ifdef debug
	    printf("\n\t(%s): Redirect stderr (append) to %s.\n",
		    __func__,
		    iopath);
#endif
	    err_append(iopath);
	    return 1;
	}
	else if (strcmp(tokenized[lastpos - 2], "2>") == 0)
	{
#ifdef debug
	    printf("\n\t(%s): Redirect stderr (overwrite) to %s.\n",
		    __func__,
		    iopath);
#endif
	    err_overwrite(iopath);
	    return 1;
	}
	else if (strcmp(tokenized[lastpos - 2], "<") == 0)
	{
#ifdef debug
	    printf("\n\t(%s): Redirect stdin from %s.\n", 
		    __func__,
		    iopath);
#endif
	    in(iopath);
	    return 1;
	}
    }

    return 0;
}

static void out_append(char *path)
{
    int new_out = open(path, O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
    ensure_open(new_out);
    ensure_dup2(dup2(new_out, STDOUT_FILENO));
}


static void out_overwrite(char *path)
{
    int new_out = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    ensure_open(new_out);
    ensure_dup2(dup2(new_out, STDOUT_FILENO));
}


static void in(char *path)
{
    int new_in = open(path, O_RDONLY);
    ensure_open(new_in);
    ensure_dup2(dup2(new_in, STDIN_FILENO));
}


static void err_append(char *path)
{
    int new_err = open(path, O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
    ensure_open(new_err);
    ensure_dup2(dup2(new_err, STDERR_FILENO));
}


static void err_overwrite(char *path)
{
    int new_err = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    ensure_open(new_err);
    ensure_dup2(dup2(new_err, STDERR_FILENO));
}


