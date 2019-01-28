/*
 * buildrooms.c
 * author: Alexander Nead-Work
 * class/prof: CS344, W2019, Justin Goins
 *
 * compile with `make buildrooms`
 * or `gcc neadwora.buildrooms.c -o neadwora.buildrooms`
 *
 * usage: ./neadwora.buildrooms
 *
 * resources used:
 * - https://stackoverflow.com/a/7430277 (make a directory)
 * - https://stackoverflow.com/a/25799033 (strings + functions)
 */


#ifndef BUILDROOMS_H
#define BUILDROOMS_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DIR_STUB "neadwora.rooms"
#define DIR_LENGTH 21 // onid=8, rooms=5, pid=5 (max), 2x.=2, \0=1, total=21

// assignment spec says all rooms must have [3,6] connections
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6

char* room_names[10] = { "SKY", "EARTH", "CAVERN", "JAIL", "CASTLE",\
			 "CEMETERY", "MARKET", "LAVA", "LASERS", "ICE" };
char* room_types[3] = { "START_ROOM", "END_ROOM", "MID_ROOM" }; // from assignment spec

typedef struct room {
    char* name;
    char* type;
    struct room* connections[MAX_CONNECTIONS];
    int num_connections;
} Room;

int log_info(char *);
int log_error(char *);

int get_directory_name(char *);
int make_directory(char *);

#endif


int main() {
    // make directory
    char dir_name[DIR_LENGTH];

    if (!get_directory_name(dir_name)) {
	log_error("couldn't generate directory name, exiting");
	return 1;
    }

    if (!make_directory(dir_name)) {
	log_error("couldn't make directory, exiting");
	return 1;
    }

    log_info("made directory:");
    log_info(dir_name);
    
    // generate rooms
    Room rooms[7];

    // make connections

    // serialize

    return 0;
}

// write log message to stdout (debug mode only)
int log_info(char *msg) {
#ifdef DEBUG
    return printf("[*] %s\n", msg);
#else
    return 0;
#endif
}

// writes error message to stderr
int log_error(char *msg) {
    return fprintf(stderr, "[!] %s\n", msg);
}

// generates directory name
// args:
//  buf: char[DIR_LENGTH]
int get_directory_name(char *buf) {
    int pid = getpid();

    memset(buf, 0, DIR_LENGTH);
    return snprintf(buf, DIR_LENGTH, "%s.%d", DIR_STUB, pid);
}

// make directory named $name
int make_directory(char *name) {
    // mkdir returns 0 if successful
    // 0 is false in C, so flip it
    return !mkdir(name, 0700);
}
