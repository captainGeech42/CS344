#ifndef SMALLSH_H
#define SMALLSH_H

#include "builtins.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LINE 2048
#define MAX_NUM_ARGS 512

int proc_status = 0; // this is set for the foreground process exit code
bool fin = false; // main loop condition

typedef struct command {
    char *line; // full command input
    char *program; // command to run
    int argc; // num args
    char **argv; // args
    char *input_file; // input file (optional)
    char *output_file; // output file (optional)
    bool background; // true if & (optional)
} Command;

int main();

Command* alloc_cmd();
void free_cmd(Command *);

void prompt(Command *);
bool parse(Command *);
void run(Command *);

#endif