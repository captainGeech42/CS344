#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct bkgds {
    int num;
    pid_t *pids;
} BackgroundProcs;

bool foreground_mode; //^Z to toggle foreground-only mode
BackgroundProcs *bkgd_procs;

void add_background_proc(pid_t);




void register_child();
void handle_sigint_child(int);


void register_parent();
void handle_sigint_parent(int);


void handle_sigint(int);
void handle_sigtstp(int);

#endif