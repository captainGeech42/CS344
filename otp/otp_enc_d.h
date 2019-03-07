#ifndef OTP_ENC_D_H
#define OTP_ENC_D_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define CONNECTION_BACKLOG 5
#define CONNECTION_TYPE "encode"
#define BUFFER_BASE 1024

int main(int, char **);
void process(int);
int recv_data(int, char *, char *);
void send_ciphertext(int, const char *, const char *);
int get_int(char);
char get_char(int);

#endif