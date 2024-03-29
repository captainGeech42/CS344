/*
 * adventure.c
 * author: Alexander Nead-Work
 * class/prof: CS344, W2019, Justin Goins
 *
 * compile with `make adventure`
 * or `gcc neadwora.adventure.c -o neadwora.adventure`
 *
 * usage: ./neadwora.adventure
 *
 * resources used:
 * - https://stackoverflow.com/a/3554147 (get directory contents)
 * - https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html (dirent fields)
 * - https://stackoverflow.com/a/4559940 (stdarg)
 * - https://www.gnu.org/software/libc/manual/html_node/Testing-File-Type.html (check if dir)
 * - https://stackoverflow.com/a/1121438 (get num files in dir)
 * - https://stackoverflow.com/a/28462221 (remove trailing newline from string)
 * - https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm (strftime)
 * - https://stackoverflow.com/a/1157217 (nanosleep)
 */


#ifndef ADVENTURE_H
#define ADVENTURE_H

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// assignment spec says dir is "onid.rooms.pid"
#define DIR_STUB "neadwora.rooms."
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
#define MAX_ROOM_NAME 10 // max length of room name + \0
#define MAX_ROOM_TYPE 12 // max length of room type + \0
char* room_names[NUM_ROOM_NAMES] = { "SKY", "EARTH", "CAVERN", "JAIL", "CASTLE",\
			 "CEMETERY", "MARKET", "LAVA", "LASERS", "ICE" };
char* room_types[NUM_ROOM_TYPES] = { "START_ROOM", "END_ROOM", "MID_ROOM" }; // from assignment spec

typedef struct room {
    char *name;
    char *type;
    char **connections;
    int num_connections;
} Room;

// start with 8 rooms allocated for the path
#define STARTING_ROOMS_PATH 8
typedef struct path {
    int num_rooms;
    int max_rooms;
    Room **rooms;
} Path;

// assignment spec says timestamp written to currentTime.txt
#define TS_PATH "currentTime.txt"

void log_info(const char *);
void log_infof(const char *, ...);
void log_error(const char *);
void log_errorf(const char *, ...);

void get_room_dir(char *);
Room** room_alloc(size_t);
Room** deserialize(const char *);
void room_free(Room **, size_t);

Path* path_alloc();
void add_room_to_path(Path *, Room *);
void print_path(Path *);
void path_free(Path *);

Room* get_start(Room **);
Room* get_room(Room **, const char *);
bool is_valid_move(Room *, const char *);
void prompt(Room *, bool);
void get_command(Room *, char *, size_t, bool);
void run_game(Room **);

void* run_time(void *);
void my_sleep(int);

#endif


int main() {
    // get room dir
    char dir[DIR_LENGTH];
    get_room_dir(dir);
    log_infof("room dir: %s\n", dir);

    // deserialize rooms
    Room **rooms = deserialize(dir);

    // run game
    run_game(rooms);

    // free
    room_free(rooms, NUM_ROOMS);

    return 0;
}

// get the most recent room directory
void get_room_dir(char *dir) {
    // open current directory
    DIR* cwd = opendir(".");
    if (!cwd) {
        log_error("unable to open cwd, exiting");
        exit(1);
    }

    // loop through contents of the directory
    struct dirent* dent;
    struct stat st, most_recent;

    // loop through all files in cwd
    while ((dent = readdir(cwd)) != NULL) {
        // get stat
        if (stat(dent->d_name, &st) == 0) {
            // check if dir, then check if room dir
            if (S_ISDIR(st.st_mode) && strstr(dent->d_name, DIR_STUB)) {
                // check time
                if (most_recent.st_mtime < st.st_mtime) {
                    log_infof("got new most recent: %s\n", dent->d_name);
                    
                    // save stat
                    most_recent = st;

                    // save dir name
                    memset(dir, 0, DIR_LENGTH);
                    strncpy(dir, dent->d_name, strlen(dent->d_name));
                }
            }
        } else {
            log_errorf("unable to stat file: %s\n", dent->d_name);
        }
    }

    // close cwd
    closedir(cwd);
}

// alloc memory for room array (free with room_free())
Room** room_alloc(size_t n) {
    // alloc array of rooms
    Room **rooms = malloc(sizeof(Room *) * n);

    // alloc each room
    int i, j;
    for (i = 0; i < n; i++) {
        rooms[i] = malloc(sizeof(Room));

        // alloc name
        rooms[i]->name = malloc(sizeof(char) * MAX_ROOM_NAME);

        // alloc type
        rooms[i]->type = malloc(sizeof(char) * MAX_ROOM_TYPE);

        // alloc array of connections
        rooms[i]->connections = malloc(sizeof(char *) * MAX_CONNECTIONS);
        
        // alloc each connection
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            rooms[i]->connections[j] = malloc(sizeof(char) * MAX_ROOM_NAME);
            memset(rooms[i]->connections[j], 0, MAX_ROOM_NAME);
        }
    }

    return rooms;
}

// read in rooms from disk, and generate room array
Room** deserialize(const char *dir) {
    // cd to room dir
    chdir(dir);

    // open room dir
    DIR *room_dir = opendir(".");
    if (!room_dir) {
        log_errorf("unable to open room dir (%s), exiting...\n", dir);
        exit(1);
    }

    // malloc
    Room **rooms = room_alloc(NUM_ROOMS);

    struct dirent *dent;
    FILE *room_file;
    char buf[100];
    memset(buf, 0, 100);
    int room_counter = 0;
    // read in each file
    while ((dent = readdir(room_dir)) != NULL) {
        if (strstr(dent->d_name, ".room")) {
            // file has .room extension
            log_infof("got room file %s\n", dent->d_name);

            // open room file
            room_file = fopen(dent->d_name, "r");
            if (room_file == NULL) {
                log_errorf("unable to open file: %s\n", room_file);
            }

            // set name
            fscanf(room_file, "ROOM NAME: %s", buf);
            strcpy(rooms[room_counter]->name, buf);
            // rooms[room_counter]->name = buf;
            log_infof("got room name: %s\n", rooms[room_counter]->name);

            // set connections
            // this also will set room type
            int connection_counter = 0;
            while (fgets(buf, 100, room_file) != NULL) {
                if (strstr(buf, "CONNECTION")) {
                    // we have a connection line
                    sscanf(buf, "CONNECTION %d: %s", &connection_counter, buf);
                    // rooms[room_counter]->connections[--connection_counter] = buf;
                    strcpy(rooms[room_counter]->connections[--connection_counter], buf);
                    log_infof("got connection: %s\n", rooms[room_counter]->connections[connection_counter]);
                } else if (strstr(buf, "ROOM TYPE")) {
                    // we have a room type line
                    sscanf(buf, "ROOM TYPE: %s", buf);
                    strcpy(rooms[room_counter]->type, buf);
                    log_infof("got room type: %s\n", rooms[room_counter]->type);
                }
            }

            // close file
            fclose(room_file);

            // set number of connections
            rooms[room_counter]->num_connections = ++connection_counter;

            // increment counter
            room_counter++;
        }
    }

    // return
    closedir(room_dir);
    chdir("..");
    return rooms;
}

// free all room array memory
void room_free(Room **rooms, size_t n) {
    int i, j;

    for (i = 0; i < n; i++) {
        // free name
        free(rooms[i]->name);

        // free type
        free(rooms[i]->type);

        // free each connection
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            free(rooms[i]->connections[j]);
        }

        // free array of connections
        free(rooms[i]->connections);

        // free room
        free(rooms[i]);
    }

    // free array of rooms
    free(rooms);
    rooms = NULL;
}

// alloc a Path
Path* path_alloc() {
    Path *p = malloc(sizeof(Path));
    p->rooms = malloc(sizeof(Room*) * STARTING_ROOMS_PATH);
    p->num_rooms = 0;
    p->max_rooms = STARTING_ROOMS_PATH;
    return p;
}

// add a room to the path
void add_room_to_path(Path *path, Room *room) {
    if (path->num_rooms == path->max_rooms) {
        // we've used all available rooms

        // allocate new array
        Room **new_rooms = malloc(sizeof(Room*) * (path->max_rooms*2));

        // move old array data to new array
        int i;
        for (i = 0; i < path->num_rooms; i++) {
            new_rooms[i] = path->rooms[i];
        }

        // free old array
        free(path->rooms);

        // update struct with new array
        path->rooms = new_rooms;
        path->max_rooms *= 2;
    }

    // add room to the path
    path->rooms[path->num_rooms++] = room;
}

// print the name of all rooms in path
void print_path(Path *path) {
    int i;
    for (i = 0; i < path->num_rooms; i++) {
        puts(path->rooms[i]->name);
    }
}

// free all path memory
void path_free(Path *path) {
    free(path->rooms);
    free(path);
    path = NULL;
}

// return the pointer for the START_ROOM
Room* get_start(Room **rooms) {
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
        if (strcmp(rooms[i]->type, "START_ROOM") == 0) {
            return rooms[i];
        }
    }
    log_error("no start room found, exiting");
    exit(1);
}

// returns the pointer for the room with name $name
Room* get_room(Room **rooms, const char *name) {
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
        if (strcmp(rooms[i]->name, name) == 0) {
            return rooms[i];
        }
    }
    log_errorf("no room \"%s\" found, exiting\n", name);
    exit(1);
}

// checks if $name is a valid connection for $room
// returns true if valid, otherwise false
bool is_valid_move(Room *room, const char *name) {
    int i;
    for (i = 0; i < room->num_connections; i++) {
        if (strcmp(room->connections[i], name) == 0) return true;
    }
    return false;
}

// print prompt to player
void prompt(Room *room, bool full_prompt) {
    if (full_prompt) {
        // print name
        printf("CURRENT LOCATION: %s\n", room->name);

        // print connections
        printf("POSSIBLE CONNECTIONS: ");
        int i;
        for (i = 0; i < room->num_connections; i++) {
            printf("%s", room->connections[i]);

            // if not the last room, print a comma, otherwise a period
            if (i+1 != room->num_connections) {
                printf(", ");
            } else {
                puts(".");
            }
        }
    }

    // print prompt
    printf("WHERE TO? >");
}

// get command from player
void get_command(Room *room, char *buf, size_t n, bool full_prompt) {
    bool valid = false;

    while (!valid) {
        // get user input
        fgets(buf, n, stdin);
        buf[strcspn(buf, "\n")] = 0;

        if (is_valid_move(room, buf) || strcmp(buf, "time") == 0) {
            valid = true;
        }

        if (!valid) {
            puts("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
            prompt(room, full_prompt);
        }
    }
}

void run_game(Room **rooms) {
    Room *current_room = get_start(rooms);
    Path *path = path_alloc();
    FILE *fp;
    char buf[100];
    memset(buf, 0, 100);
    bool full_prompt = true;

    // initialize time thread
    pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&time_mutex);
    pthread_t time_thread;
    pthread_create(&time_thread, NULL, run_time, &time_mutex);

    // main game loop
    while (true) {
        // print prompt
        prompt(current_room, full_prompt);

        // get command
        get_command(current_room, buf, 100, full_prompt);

        // if room, move
        if (strcmp(buf, "time") != 0) {
            // not handling time, move rooms
            current_room = get_room(rooms, buf);
            full_prompt = true;

            // add room to path
            add_room_to_path(path, current_room);

            // blank line
            puts("");
        } else {
            // handling time command
            pthread_mutex_unlock(&time_mutex);
            log_info("game thread, waiting for lock");
            // wait for time thread to grab lock
            my_sleep(25);
            pthread_mutex_lock(&time_mutex);
            log_info("game thread, got lock");

            // get time from file
            fp = fopen(TS_PATH, "r");
            fgets(buf, 100, fp);

            // print time
            printf("\n %s\n", buf);

            // time command doesn't need the rooms outputted again
            full_prompt = false;
        }

        // check if in end room
        if (strcmp(current_room->type, room_types[1]) == 0) {
            puts("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
            printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", path->num_rooms);
            print_path(path);

            break;
        }
    }

    // kill time thread
    pthread_cancel(time_thread);

    // free path
    path_free(path);
}

// code for time thread
void* run_time(void *mutex) {
    time_t ts;
    struct tm *tinfo;
    char buf[100];
    FILE *ts_file;

    while (true) {
        // get lock
        log_info("time thread, waiting for lock");
        pthread_mutex_lock(mutex);
        log_info("time thread, got lock");

        // get time
        time(&ts);
        tinfo = localtime(&ts);
        // 1:03pm, Tuesday, September 13, 2016
        strftime(buf, 100, "%I:%M%P, %A, %B %d, %Y", tinfo);

        // print time
        // printf("\n %s\n\n", buf);
        // turns out you have to read it from the file it gets written to, rip

        // write to file
        ts_file = fopen(TS_PATH, "w");
        fprintf(ts_file, "%s\n", buf);
        fclose(ts_file);

        // unlock
        pthread_mutex_unlock(mutex);

        // let game thread grab lock
        my_sleep(25);
    }
}

// sleep for $ms milliseconds
void my_sleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}



/**** Debug/error printing ****/

// write log message to stdout (debug mode only)
void log_info(const char *msg) {
#ifndef DEBUG
    return;
#endif

    printf("[*] %s\n", msg);
}

// write log messages to stdout w/ format string (debug mode only)
void log_infof(const char *fmt, ...) {
#ifndef DEBUG
    return;
#endif

    // make new fmt str with header
    int new_fmt_len = strlen(fmt) + 5;
    char new_fmt[new_fmt_len];
    snprintf(new_fmt, new_fmt_len, "[*] %s", fmt);

    // print
    va_list args;
    va_start(args, fmt);
    vprintf(new_fmt, args);
    va_end(args);
}

// writes error message to stderr
void log_error(const char *msg) {
    fprintf(stderr, "[!] %s\n", msg);
}

// write error messages to stderr w/ format string
void log_errorf(const char *fmt, ...) {
    // make new fmt str with header
    int new_fmt_len = strlen(fmt) + 5;
    char new_fmt[new_fmt_len];
    snprintf(new_fmt, new_fmt_len, "[!] %s", fmt);

    // print
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, new_fmt, args);
    va_end(args);
}