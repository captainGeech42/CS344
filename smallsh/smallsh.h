#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LINE 2048
#define MAX_NUM_ARGS 512

extern int errno;
bool exit = false;

typedef struct command {
    char *line;
    char *program;
    char **argv;
    int argc;
} Command;

int main(int, char **);

Command* alloc_cmd();
void free_cmd(Command *);

void prompt(char **);

#endif