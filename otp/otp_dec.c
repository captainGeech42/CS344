#include "otp_dec.h"

int main(int argc, char **argv) {
    // validate arguments
    if (argc != 4) {
        fprintf(stderr, "usage: %s [ciphertext] [key] [port]\n", argv[0]);
        return 1;
    }

    // parse port
    int port = atoi(argv[3]);
    if (port <= 1024) {
        fputs("invalid port (must be > 1024)\n", stderr);
        return 1;
    }

    // read in plaintext and key
    char *plaintext, *key;
    int plaintext_size = get_file(argv[1], &plaintext);
    int key_size = get_file(argv[2], &key);

    // make sure we have a valid key length
    if (strlen(key) < strlen(plaintext)) {
        fprintf(stderr, "Error: key '%s' is too short'\n", argv[2]);
        return 1;
    }

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

    if (!handshake(socket_fd)) {
        fputs("couldn't connect to otp_enc_d, bad port?\n", stderr);
        return 1;
    }

    send_data(socket_fd, plaintext, plaintext_size);
    send_data(socket_fd, key, key_size);

    char *ciphertext;
    recv_data(socket_fd, &ciphertext);

    close(socket_fd);
}

// get the contents of a file, save to *data, return length of buffer in data
int get_file(const char *filename, char **data) {
    // TODO add error checking
    int fd = open(filename, O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    int pagesize = getpagesize();
    int size = st.st_size + (pagesize - (st.st_size % pagesize));
    *data = (char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    return size;
}

// handshake with the server
bool handshake(int socket) {
    // tell server we are encode
    if (write(socket, CONNECTION_TYPE, strlen(CONNECTION_TYPE) + 1) < 0) {
        fputs("error handshake: write\n", stderr);
        return false;
    }

    char recvd[10];
    memset(recvd, 0, 10);
    if (read(socket, recvd, strlen(CONNECTION_GOOD) + 1) < 0) {
        fputs("error handshake: read\n", stderr);
        return false;
    }

    if (strcmp(recvd, CONNECTION_GOOD) != 0) {
        fputs("error handshake: validate\n", stderr);
        return false;
    }

    return true;
}

// send data to the server
bool send_data(int socket, char *data, int size) {
    // align data to BUFFER_BASE byte blocks
    size_t newlen;
    char *newdata = malloc(sizeof(char) * size+1);
    memset(newdata, 0, size+1);
    strncpy(newdata, data, size);
    data = newdata;
    if (size % BUFFER_BASE != 0) {
        newlen = size + (BUFFER_BASE % size);
        char *new = malloc(sizeof(char) * newlen);
        memset(new, 0, newlen);
        strcpy(new, data);
        data = new;
    }

    for (int i = 0; i < strlen(data); i += BUFFER_BASE) {
        if (write(socket, data+i, BUFFER_BASE) < 0) {
            fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
            return false;
        }
    }

    // free(data);
    return true;
}

// recv ciphertext from server
ssize_t recv_data(int socket, char **ciphertext) {
    char *data;
    // alloc base memory for ciphertext
    data = malloc(sizeof(char) * BUFFER_BASE);
    memset(data, 0, BUFFER_BASE);

    // get data from the socket
    ssize_t bytes_read = read(socket, data, BUFFER_BASE);
    if (bytes_read < 0) {
        fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
        return -1;
    }
#ifdef DEBUG
    else {
        puts(data);
        //printf("got %zu bytes\n", bytes_read);
    }
#endif

    // check if we got the end of the data
    int base_count = 1;
    char *new;
    while (strpbrk(data, "\n") == NULL) {
        // there is more data to be read
        
        // copy over old plaintext to new buffer
        new = malloc(sizeof(char) * (BUFFER_BASE * (++base_count)));
        memset(new, 0, BUFFER_BASE*base_count);
        strcpy(new, data);
        free(data);
        data = new;

        // read in more data
        bytes_read = read(socket, data+(BUFFER_BASE*(base_count-1)), BUFFER_BASE);
        if (bytes_read < 0) {
            fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
            return -1;
        }
#ifdef DEBUG
        else {
            puts(data)n ;
            //printf("got %zu bytes\n", bytes_read);
        }
#endif
    }

    printf("%s", data);
    ciphertext = &data;
    return bytes_read;
}