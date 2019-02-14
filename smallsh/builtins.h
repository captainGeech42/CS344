#ifndef BUILTINS_H
#define BUILTINS_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void my_exit();
void my_cd(const char *);
void my_status(int);

#endif