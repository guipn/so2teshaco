#ifndef BUILTIN_CMD
#define BUILTIN_CMD

void my_exit(void);
int  cd(char *);
void show_jobs(void);
void bg(char *);
void fg(char *);
int  run_builtin(char *);

#endif

