#ifndef OTP_ENC_D_H
#define OTP_ENC_D_H

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
void encode(char *, const char *);

#endif