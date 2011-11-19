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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


/**
 * $name make_job;
 * $proto job_t *make_job(pid_t pid, char *name, enum job_state status, char fg);
 *
 * Cria um job para o shell.
 * pid eh o pid do novo job sendo registado.
 * name eh o nome do job.
 * status eh seu estado (vide jobs.h).
 * fg deve ser 0, se o job estiver sendo executado em foreground, e != 0 caso contrario.
 */

job_t *make_job(pid_t pid, char *name, enum job_state status, char fg)
{
    job_t *j = _malloc(sizeof *j);

    j->pid        = pid;
    j->status     = status;
    j->foreground = fg;
    
    /* fix! */
    strcpy(j->name, name);

    return j;
}


/**
 * $name make_job_node;
 * $proto job_node_t *make_job_node(job_t *job);
 *
 * Cria, dado um job, um no que pode ser adicionado a uma lista de
 * nos contendo outros jobs.
 */

job_node_t *make_job_node(job_t *job)
{
    job_node_t *jn = _malloc(sizeof *jn);

    jn->job  = job;
    jn->next = NULL;
    jn->prev = NULL;

    return jn;
}



/**
 * $name get_fg_job;
 * $proto job_t *get_fg_job(job_node_t *list);
 *
 * Retorna o job que estiver em foreground.
 */

job_t *get_fg_job(job_node_t *list)
{
    while (list && !list->job->foreground)
    {
	list = list->next;
    }

    return (list) ? list->job : NULL;
}


/**
 * $name get_job;
 * $proto job_t *get_job(job_node_t *list, pid_t pid);
 *
 * Retorna o job pertencente aa lista dada cujo pid eh igual
 * ao pid fornecido.
 */

job_t *get_job(job_node_t *list, pid_t pid)
{
    while (list && list->job->pid != pid)
    {
	list = list->next;
    }

    return (list) ? list->job : NULL;
}


/**
 * $name delete_job;
 * $proto void delete_job(job_node_t **head, pid_t pid);
 *
 * Remove o no cujo pid eh o fornecido de uma lista 
 * cujo endereco do primeiro elemento eh dado por head. 
 */

void delete_job(job_node_t **head, pid_t pid)
{
    if (*head && (*head)->job->pid == pid)
    {
	job_node_t *del = *head;
	*head           = (*head)->next;

	if (*head)
	    (*head)->prev = NULL;

	free(del->job);
	free(del);
	return;
    }

    job_node_t *p = *head;

    while (p && p->job->pid != pid)
	p = p->next;

    if (p && p->job->pid == pid)
    {
	if (p->next)
	    p->next->prev = p->prev;

	if (p->prev)
	    p->prev->next = p->next;

	free(p->job);
	free(p);
    }
}


/**
 * $name add_job
 * $proto job_node_t *add_job(job_node_t *head, job_t *new);
 * 
 * Adiciona um job aa lista que comeca em head.
 */

job_node_t *add_job(job_node_t *head, job_t *new)
{
    job_node_t *newnode = make_job_node(new);

    if (!head)
    {
	return newnode;
    }

    job_node_t *iter = head;

    while (iter->next)
	iter = iter->next;

    newnode->prev = iter;
    iter->next    = newnode;

#ifdef debug
	printf("\n\t(%s): job %d added after %d.\n", __func__, new->pid, iter->job->pid);
#endif

    return head;
    
}


/**
 * $name destroy_job_list;
 * $proto void destroy_job_list(job_node_t *head);
 *
 * Libera todo o espaco ocupado pela lista de nos (bem como o espaco
 * utilizado por cada job contido em cada no).
 */

void destroy_job_list(job_node_t *head)
{
    if (!head)
	return;

    job_node_t *following = head->next;

    while (following)
    {
	kill(head->job->pid, SIGKILL);
	free(head->job);
	free(head);
	head      = following;
	following = following->next;
    }
}


/**
 * $name print_jobs;
 * $proto void print_jobs(job_node_t *head);
 *
 * Imprime todos os nos contidos na lista que comeca em head.
 */

void print_jobs(job_node_t *head)
{
    if (!head)
    {
	puts("\n\tNao ha jobs em execucao.");
	return;
    }

    puts(
	    " [Nome do Job]\t\t[pid]\t\t[status]\t\t[foreground]\n"
	    " -------------\t\t-----\t\t--------\t\t------------\n"
	);

    char *job_status_names[] = 
    {
	"Executando", "Pausado", "Terminado"
    };

    while (head)
    {
	printf(" %10.10s\t\t%u\t\t%-10.10s\t\t%-10.10s\n", 
		head->job->name,
		head->job->pid,
		job_status_names[head->job->status],
		head->job->foreground ? "Sim" : "Nao");

	head = head->next;
    }
    puts("");
}


/**
 * $name update_status;
 * $proto void update_status(job_t *job, int status);
 *
 * Atualiza o status de um job, dado o estado fornecido pelo segundo 
 * argumento, que deve ter sido modificado por wait ou waitpid.
 */

void update_status(job_t *job, int status)
{
    if (!job)
    {
#ifdef debug
	printf("\n\t(%s): Received NULL\n\n", __func__);
#endif
	return;
    }

    if (WIFSTOPPED(status))
    {
	job->status     = paused;
	job->foreground = 0;
#ifdef debug
	printf("\n\t(%s): %d was paused.\n", __func__, job->pid);
#endif
    }
    else if (WIFCONTINUED(status))
    {
	job->status = running;
#ifdef debug
	printf("\n\t(%s): %d was continued.\n", __func__, job->pid);
#endif
    }
    else 
    {
	job->status     = finished;
	job->foreground = 0;
#ifdef debug
	printf("\n\t(%s): %d has finished.\n", __func__, job->pid);
#endif
    }
#ifdef debug
    puts("");
#endif
}


/**
 * $name invoke_bg;
 * $proto void invoke_bg(job_node_t *list, pid_t pid);
 *
 * Ativa um job e o leva para background. 
 */

void invoke_bg(job_node_t *list, pid_t pid)
{
    job_t *job = get_job(list, pid);

    if (!job)
    {
	fprintf(stderr, "\n\t** Nao ha jobs com o pid determinado (%d). **\n\n", pid);
	return;
    }

    if (job->status == finished)
    {
	fprintf(stderr, "\n\t** O job com pid %d ja terminou. **\n\n", pid);
	return;
    }

    job->foreground = 0;
    job->status     = running;

    kill(job->pid, SIGCONT);
}


/**
 * $name invoke_fg;
 * $proto void invoke_fg(job_node_t *list, pid_t pid);
 *
 * Ativa um job (que nao tenha terminado) e o traz para foreground.
 */

void invoke_fg(job_node_t *list, pid_t pid)
{
    job_t *job = get_job(list, pid);

    if (!job)
    {
	fprintf(stderr, "\n\t** Nao ha jobs com o pid determinado (%d). **\n\n", pid);
	return;
    }
    if (job->status == finished)
    {
	fprintf(stderr, "\n\t** O job com pid %d ja terminou. **\n\n", pid);
	return;
    }
    if (job->foreground)
    {
	fprintf(stderr, "\n\t** O job com pid %d ja esta rodando em foreground. **\n\n", pid);
	return;
    }

    job->foreground = 1;
    job->status     = running;

    kill(job->pid, SIGCONT);
}

