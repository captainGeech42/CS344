#ifndef OTP_DEC_D_H
#define OTP_DEC_D_H

#include "common.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int main(int, char **);
void process(int);
void decode(char *, const char *);

#endif