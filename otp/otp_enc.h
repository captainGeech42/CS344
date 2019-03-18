#ifndef OTP_ENC_H
#define OTP_ENC_H

#include "constants.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define CONNECTION_TYPE "encode"

int main(int, char **);
int get_file(const char *, char **);
bool handshake(int);
bool send_data(int, char *, int);
ssize_t recv_data(int, char **);

#endif