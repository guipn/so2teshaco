#include "builtincmd.h"
#include "io.h"
#include "jobs.h"
#include "memory.h"
#include "sig.h"
#include "str.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>

#define SHELL_NAME "teshaco"
#define MAX_PIPES  100


char        **path;
job_node_t  *jobs;


void errexit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/**
 * $name print_shell_name;
 * $proto void print_shell_name(void);
 *
 * Exibe o nome do shell (teshaco) para esperar pelo input
 * do usuario.
 */

void print_shell_name(void)
{
    char cwd[512];

    if (!getcwd(cwd, sizeof cwd))
	printf("%s> ", SHELL_NAME);
    else
	printf("%s:%s> ", SHELL_NAME, cwd);
}


/**
 * $name call_exec;
 * $proto void call_exec(char **tokenized);
 *
 * Substitui o processo atual. tokenized deve ser o retorno de
 * split(cmd, " ");, sendo cmd uma linha de comando que nao contenha '&'.
 *
 * Exemplo: 
 *
 * strcpy(buffer, "ls /");
 * run_exec(split(buffer, " "));
 */

void call_exec(char **tokenized)
{ 
    if (!path)
	path = parse_PATH();

    if (redirect_io(tokenized))
    {
	char **delete = tokenized + string_buffer_length(tokenized) - 2;
	free(delete[0]), free(delete[1]); 
	delete[0] = NULL; // mark it for free_strings call in run_os
    }

    int i = 0;

    for (; path[i]; i++)
    {
	size_t len = strlen(path[i]) + strlen(*tokenized);
	char try_path[len + 2];

	strcpy(try_path, path[i]);
	strcat(try_path, "/");
	strcat(try_path, *tokenized);
	execv(try_path, tokenized);
    }
}



/**
 * $name run_os;
 * $proto void run_os(char *cmd);
 *
 * Processa um comando, determinando se ele deve ser executado
 * em foreground ou em background.
 *
 * Quebra a linha de comando nas pipes, criando um vetor de comandos
 * unicos. Chama a syscall pipe para fazer a comunicacao entre os 
 * processos que vao sendo criados aa medida em que o vetor de comandos
 * eh percorrido.
 * 
 */

void run_os(char *cmd)
{
    char **commands = split(cmd, "|");
    int i = 0, pipefd[100][2];

    for (; commands[i]; i++)
    {
	if (i > MAX_PIPES - 1)
	{
	    fprintf(stderr, "\n\tO numero maximo permitido de pipes eh 100.\n");
	    return;
	}

	int foreground;
	char **tokenized = get_command_tokens(commands[i], &foreground);

	if (pipe(pipefd[i]) == -1)
	    errexit("pipe");

	if (i < MAX_PIPES - 1 && commands[i + 1] && pipe(pipefd[i + 1]) == -1)
	    errexit("pipe2");

	pid_t pid;
	job_t *newjob;
	switch ((pid = fork()))
	{
	    case -1:
		errexit("fork");
		break; 

	    case 0:
		if (i < MAX_PIPES - 1 && commands[i + 1])
		{
		    // Write to pipe 
		    dup2(pipefd[i][1], STDOUT_FILENO);
		}

		if (i != 0)
		{
		    // Read from last process' reading end
		    dup2(pipefd[i - 1][0], STDIN_FILENO);
		}

		call_exec(tokenized);

		fprintf(stderr, "\n\tComando nao reconhecido.\n\n");
		exit(EXIT_FAILURE);
		break;

	    default:
		setpgid(pid, 0);
		newjob = make_job(pid, *tokenized, running, foreground);
		(jobs == NULL) ? jobs = make_job_list(newjob) : add_job(jobs, newjob);
		close(pipefd[i][1]);

		if (foreground)
		{
#ifdef debug
		    printf("(%s): Waiting for foreground job %d to finish.\n", __func__, pid);
#endif
		    waitpid(pid, NULL, 0);
		}
		break;
	}
    }
    
    free(commands);
}



int main(void)
{
    if (atexit(release_resources) != 0)
	fprintf(stderr, "\n\t*** Erro ao registrar rotina de limpeza."
			" Pode haver vazamento de memoria. *** \n\n");

   sig_setup();

    for (;;)
    {
	puts("");
	print_shell_name();
	char *cmd = read_line(stdin);

	if (*cmd == '\0') 
	{
	    free(cmd);
	    continue;
	}

	if (run_builtin(cmd) == 0)
	    run_os(cmd);

	free(cmd);
    }
    return 0; 
}

