#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LINE 2048
#define MAX_NUM_ARGS 512

extern int errno;
bool fin = false;

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

#endif