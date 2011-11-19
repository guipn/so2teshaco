#include "jobs.h"
#include "sig.h"

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


extern job_node_t *jobs;  // main.c

static struct sigaction action_TSTP;
static struct sigaction old_TSTP;

static struct sigaction action_CHLD;
static struct sigaction old_CHLD;


/**
 * $name handle_TSTP;
 * $proto void handle_TSTP(int sig, siginfo_t *info, void *context);
 *
 * Responsavel por tratar SIGTSTP.
 */

void handle_TSTP(int sig, siginfo_t *info, void *context)
{
    (void) sig;
    (void) info;
    (void) context;

    job_t *fgjob = get_fg_job(jobs);
   
    if (!fgjob)
    {
#ifdef debug
	printf("\n\t(%s): No jobs running in foreground to send SIGSTOP.\n", __func__);
#endif
	return;
    }
    else
    {
	fgjob->foreground = 0;
	
	if (fgjob->status != finished)
	{
	    fgjob->status = paused;
#ifdef debug
	    printf("\n\t(%s): Sending SIGSTOP to %d.\n", __func__, fgjob->pid);
#endif
	    kill(fgjob->pid, SIGSTOP);
	}
    }

}


/**
 * $name handle_CHLD;
 * $proto void handle_CHLD(int sig, siginfo_t *info, void *context);
 *
 * Responsavel por tratar SIGCHLD. Usa o parametro info para
 * determinar o pid do processo filho que sofreu mudanca de estado.
 * Nao utiliza o terceiro parametro.
 */

void handle_CHLD(int sig, siginfo_t *info, void *context)
{
    (void) sig;
    (void) context;

#ifdef debug
    printf("\n\t(%s): Processing SIGCHLD from %d.\n", __func__, info->si_pid);
#endif

    int status;
    waitpid(info->si_pid, &status, WUNTRACED | WCONTINUED);
    update_status(get_job(jobs, info->si_pid), status);
}



/**
 * $name sig_setup;
 * $proto void sig_setup(void);
 *
 * Instala os handlers para os devidos sinais. Caso uma ou mais
 * instalacoes falhem, avisos especificos sao emitidos.
 */

void sig_setup(void)
{
    sigemptyset(&action_TSTP.sa_mask);
    action_TSTP.sa_flags     = SA_SIGINFO;
    action_TSTP.sa_sigaction = handle_TSTP;

    if (sigaction(SIGTSTP, &action_TSTP, &old_TSTP) == -1)
	fprintf(stderr, "\n\tErro ao tentar instalar handler para SIGTSTP."); 


    sigemptyset(&action_CHLD.sa_mask);
    action_CHLD.sa_flags     = SA_SIGINFO;
    action_CHLD.sa_sigaction = handle_CHLD;

    if (sigaction(SIGCHLD, &action_CHLD, &old_CHLD) == -1)
	fprintf(stderr, "\n\tErro ao tentar instalar handler para SIGCHLD."); 
}


/**
 * $name children_sig_setup;
 * $proto void children_sig_setup(void);
 *
 * Restaura os handlers anteriores aa execucao de sig_setup(). Para ser
 * usada nos processos filhos do shell.
 */

void children_sig_setup(void)
{
    sigaction(SIGTSTP, &old_TSTP, NULL);
    sigaction(SIGCHLD, &old_CHLD, NULL);
}

