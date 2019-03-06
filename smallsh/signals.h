#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

bool foreground_mode; //^Z to toggle foreground-only mode

void register_child();
void handle_sigint_child(int);
void handle_sigtstp_child(int);

void register_parent();
void handle_sigint_parent(int);
void handle_sigtstp_parent(int);

#endif