#ifndef COMMON_H
#define COMMON_H

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

#define CONNECTION_BACKLOG 5
// #define BUFFER_BASE 1024
#define MAX_DATA 70002 // 70k + newline + nullbyte

#ifdef ENCODE
#define HANDSHAKE_GOOD 0xaa
#define HANDSHAKE_BAD 0xbb
#endif

#ifdef DECODE
#define HANDSHAKE_GOOD 0xcc
#define HANDSHAKE_BAD 0xdd
#endif

int get_int(char);
char get_char(int);

int get_file(const char *, char **);

int start_server(int);
int connect_to_server(int);
int recv_data(int, char **);
bool send_data(int, const char *, size_t);
bool handshake_server(int);
bool handshake_client(int);

#endif