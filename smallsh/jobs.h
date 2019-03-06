#ifndef JOBS_H
#define JOBS_H

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct bkgds {
    int num;
    pid_t *pids;
} BackgroundProcs;

BackgroundProcs *procs;
pthread_t bkgd_thread;
pthread_mutex_t proc_lock;

void jobs_init();
void* check_bkgds(void *);
void my_sleep(int);
void add_proc(pid_t);
void kill_bkgds();

#endif