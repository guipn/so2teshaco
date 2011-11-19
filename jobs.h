#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include <unistd.h>

#define JOB_NAME_LEN 512

enum job_state
{
    running, paused, finished
};

typedef struct job
{
    pid_t pid;
    char name[JOB_NAME_LEN];
    enum job_state status;
    char foreground;
} job_t;

typedef struct job_node
{
    job_t           *job;
    struct job_node *next;
    struct job_node *prev;
} job_node_t;


job_t *make_job(pid_t, char *, enum job_state, char);
job_t *get_fg_job(job_node_t *);
job_t *get_job(job_node_t *, pid_t);
job_node_t *make_job_node(job_t *);
job_node_t *add_job(job_node_t *, job_t *);
void delete_job(job_node_t **, pid_t);
void destroy_job_list(job_node_t *);
void print_jobs(job_node_t *);
void update_status(job_t *, int);
void invoke_bg(job_node_t *, pid_t);
void invoke_fg(job_node_t *, pid_t);

#endif

