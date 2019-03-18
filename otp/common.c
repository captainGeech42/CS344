#include "common.h"

// convert char from plaintext/key to int
int get_int(char c) {
    if (c == ' ') {
        return 26;
    } else {
        return c - 'A';
    }
}

// convert int back to char for ciphertext
char get_char(int i) {
    if (i == 26) {
        return ' ';
    } else {
        return 'A' + i;
    }
}

// get the contents of a file, save to *data, return length of buffer in data
int get_file(const char *filename, char **data) {
    // TODO add error checking
    int fd = open(filename, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    int pagesize = getpagesize();
    int size = st.st_size + (pagesize - (st.st_size % pagesize));
    *data = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    return size;
}

// start a listener on the specified port
// returns socket FD
int start_server(int port) {
    // setup server socket struct
    struct sockaddr_in server_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // setup socket
    int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket < 0) {
        fprintf(stderr, "error opening socket: %s\n", strerror(errno));
        return -1;
    }

    // bind socket
    if (bind(listener_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "error on bind: %s\n", strerror(errno));
        return -1;
    }

    // wait for clients
    if (listen(listener_socket, CONNECTION_BACKLOG) < 0) {
        fprintf(stderr, "error on listen: %s\n", strerror(errno));
        return -1;
    }

    return listener_socket;
}

// connect to server (doesn't handshake)
// returns socket FD
int connect_to_server(int port) {
    // setup socket struct
    struct sockaddr_in server_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // setup socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "error on socket: %s\n", strerror(errno));
        return 1;
    }

    // connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) < 0) {
        fprintf(stderr, "error on connect: %s\n", strerror(errno));
        return 1;
    }
}

// get data until a newline or null byte from the server
// returns length of the data
int recv_data(int fd, char **dest) {
    char *data = malloc(sizeof(char) * MAX_DATA);
    bzero(data, MAX_DATA);
    int offset = 0;

    do {
        if (read(fd, data+offset, 1) < 0) {
            fprintf("error reading from socket: %s\n", strerror(errno));
            return -1;
        }
    } while (*(data+offset) != '\n' || *(data+offset) != '\0');

    *dest = data;
    return offset;
}

// send data to the server
// returns 0 if good, 1 if bad
bool send_data(int fd, const char *src, size_t length) {
/*
#region old
*/
    // size_t newlen;
    // char *data;
    // bool mallocd;

    // // make sure our message is aligned to our block size
    // if (length % BUFFER_BASE != 0) {
    //     newlen = length + (BUFFER_BASE - length);
    //     data = malloc(sizeof(char) * newlen);
    //     mallocd = true;
    //     bzero(data, newlen);
    //     strncpy(data, src, length);
    // } else {
    //     data = src;
    //     newlen = length;
    //     mallocd = false;
    // }

    // for (int i = 0; i < newlen; i += BUFFER_BASE) {
    //     if (write(fd, data+i, BUFFER_BASE) < 0) {
    //         fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
    //         return false;
    //     }
    // }

    // if (mallocd) free(data);
/*
#endregion
*/
    
    for (int i = 0; i < length; i++) {
        if (write(fd, src+i, 1) < 0) {
            fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
        }
    }

    return true;
}

// perform handshake as server
// returns true if passed, false if not
bool handshake_server(int fd) {
    char data[2];
    bzero(data, 2);

    // get byte from client
    if (read(fd, data, 1) < 0) {
        fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
        return false;
    }

    // if we got a bad byte, fail the handshake
    if (*data != HANDSHAKE_GOOD) {
        *data = HANDSHAKE_BAD;
    }

    // reply to client
    if (write(fd, data, 1) < 0) {
        fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
        return false;
    }

    return *data == HANDSHAKE_GOOD
}

bool handshake_client(int fd) {
    char data[2];
    *data = HANDSHAKE_GOOD;

    // send byte to server
    if (write(fd, data, 1) < 0) {
        fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
        return false;
    }

    // get byte back from server
    if (read(fd, data, 1) < 0) {
        fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
        return false;
    }

    return *data == HANDSHAKE_GOOD;
}