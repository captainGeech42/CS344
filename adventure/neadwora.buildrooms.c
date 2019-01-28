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
 * - https://www.tutorialspoint.com/cprogramming/c_file_io.htm (file i/o)
 */


#ifndef BUILDROOMS_H
#define BUILDROOMS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// assignment spec says dir is "onid.rooms.pid"
#define DIR_STUB "neadwora.rooms"
#define DIR_LENGTH 21 // onid=8, rooms=5, pid=5 (max), 2x.=2, \0=1, total=21

#define MAX_FP 14 // EARTH.room; name=8 (max), .=1, room=4, \0=1, total=14

// assignment spec says all rooms must have [3,6] connections
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6

// assignment spec says 7 rooms
#define NUM_ROOMS 7

// assignment spec says 10 names, 3 types
#define NUM_ROOM_NAMES 10
#define NUM_ROOM_TYPES 3
char* room_names[NUM_ROOM_NAMES] = { "SKY", "EARTH", "CAVERN", "JAIL", "CASTLE",\
			 "CEMETERY", "MARKET", "LAVA", "LASERS", "ICE" };
char* room_types[NUM_ROOM_TYPES] = { "START_ROOM", "END_ROOM", "MID_ROOM" }; // from assignment spec

typedef struct room {
    char* name;
    char* type;
    struct room** connections;
    int num_connections;
} Room;

int log_info(char *);
int log_error(char *);

int get_directory_name(char *);
int make_directory(char *);

Room** generate_rooms();
void free_rooms(Room **);
bool is_graph_full(Room **);
void add_random_connection(Room **);
Room* get_random_room(Room **);
bool can_add_connection_from(Room *);
bool connection_already_exists(Room *, Room *);
void connect_room(Room *, Room *);
bool is_same_room(Room *, Room *);

void serialize(Room **, char *);

#endif


int main() {
    // seed rand
    srand(time(NULL));

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
    Room** rooms = generate_rooms();
    if (!rooms) {
	log_error("Unable to generate rooms, exiting");
	return 1;
    }

    // serialize
    serialize(rooms, dir_name);
    
    // free
    free_rooms(rooms);

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

// set name, type, and connections for all rooms
// args:
//  rooms: pre-allocated array for NUM_ROOMS rooms
Room** generate_rooms() {
    // alloc
    Room **rooms = malloc(sizeof(Room*) * NUM_ROOMS);
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
	rooms[i] = malloc(sizeof(Room));
	rooms[i]->connections = (Room**) malloc(sizeof(Room*) * MAX_CONNECTIONS);
	rooms[i]->num_connections = 0;
    }

    // set name
    // randomization is on the starting value, and then we take the next 7 values
    // easy way to guarentee 7 unique values
    int start = rand() % 4; // [0,3]
    for (i = 0; i < NUM_ROOM_NAMES - 3; i++) {
	rooms[i]->name = room_names[i+start];
    }

    // set type
    // first room is start
    // last room is end
    // others are mid
    rooms[0]->type = room_types[0];
    for (i = 1; i < NUM_ROOMS-1; i++) {
	rooms[i]->type = room_types[2];
    }
    rooms[NUM_ROOMS-1]->type = room_types[1];

    // set connections
    // algorithm from canvas (2.2)
    do {
	add_random_connection(rooms);
    } while (!is_graph_full(rooms));
    
    // return ptr
    return rooms;
}

// free room memory
void free_rooms(Room **rooms) {
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i]->connections);
	free(rooms[i]);
    }
    free(rooms);
    rooms = NULL;
}

// returns true if all rooms have [3,6] connections, otherwise false
// from canvas 2.2
bool is_graph_full(Room **rooms) {
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
	if (rooms[i]->num_connections < 3 || rooms[i]->num_connections > 6) {
	    return false;
	}
    }
    return true;
}

// adds a random, valid connection from one room to another
// from canvas 2.2
void add_random_connection(Room **rooms) {
    Room *r1, *r2;

    do {
	r1 = get_random_room(rooms);
    } while (!can_add_connection_from(r1));

    do {
	r2 = get_random_room(rooms);
    } while (!can_add_connection_from(r2) || is_same_room(r1, r2) || connection_already_exists(r1, r2));
    connect_room(r1, r2);
}

// return a random room
// from canvas 2.2
Room* get_random_room(Room **rooms) {
    return rooms[rand() % NUM_ROOMS];
}

// returns true if room has < 6 connections
// from canvas 2.2
bool can_add_connection_from(Room *room) {
    return room->num_connections < 6;
}

// returns true if r1 and r2 are already connected, otherwise false
// from canvas 2.2
bool connection_already_exists(Room *r1, Room *r2) {
    int i;
    for (i = 0; i < r1->num_connections; i++) {
	// since connections are bidirectional, we only have to check one room for the other
	if (strcmp(r1->connections[i]->name, r2->name)) {
	    // names are unique
	    return true;
	}
    }
    return false;
}

// add each room to the other's connections
// from canvas 2.2
void connect_room(Room *r1, Room *r2) {
    r1->connections[r1->num_connections] = r2;
    r1->num_connections++;

    r2->connections[r2->num_connections] = r1;
    r2->num_connections++;
}

// returns true if rooms are the same, otherwise false
// from canvas 2.2
bool is_same_room(Room *r1, Room *r2) {
    // names are unique
    return strcmp(r1->name, r2->name);
}

// write the rooms to disk
void serialize(Room **rooms, char *dir) {
    // set cwd
    chdir(dir);

    int i, j;
    char path[MAX_FP];
    FILE *file;
    for (i = 0; i < NUM_ROOMS; i++) {
	snprintf(path, MAX_FP, "%s.room", rooms[i]->name);
	file = fopen(path, "w");
	fprintf(file, "ROOM NAME: %s\n", rooms[i]->name);
	for (j = 0; j < rooms[i]->num_connections; j++) {
	    fprintf(file, "CONNECTION %d: %s\n", j+1, rooms[i]->connections[j]->name);
	}
	fprintf(file, "ROOM TYPE: %s\n", rooms[i]->type);
    }
}
