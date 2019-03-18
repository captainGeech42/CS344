#include "otp_dec_d.h"

int main(int argc, char **argv) {
    // check for args
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port number]\n", argv[0]);
        return 1;
    }

    // validate port
    int port = atoi(argv[1]);
    if (port <= 1024) {
        fputs("invalid port specified (must be > 1024)\n", stderr);
        return 1;
    }

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
        return 1;
    }

    // bind socket
    if (bind(listener_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "error on bind: %s\n", strerror(errno));
        return 1;
    }

    // wait for clients
    if (listen(listener_socket, CONNECTION_BACKLOG) < 0) {
        fprintf(stderr, "error on listen: %s\n", strerror(errno));
    }

    // handle clients
    int client_socket, pid, client_addr_len;
    struct sockaddr_in client_addr;
    while (true) {
        // accept client connection (blocking)
        client_addr_len = sizeof(client_addr);
        client_socket = accept(listener_socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);

        // validate socket
        if (client_socket < 0) {
            fprintf(stderr, "error accepting client connection: %s\n", strerror(errno));
            continue;
        }
#ifdef DEBUG
        puts("got client");
#endif

        // fork to handle client
        pid = fork();
        switch (pid) {
            case -1:
                fprintf(stderr, "error on fork: %s\n", strerror(errno));
                continue;
            case 0:
                // child
                close(listener_socket);
                process(client_socket);
                exit(0);
            default:
                // parent
                close(client_socket);
        }
    }
}

// get data from the connection, and send back the ciphertext
void process(int socket) {
    // verify we have an encode connection
    char conn_type[8];
    memset(conn_type, 0, 8);
    read(socket, conn_type, 7);
    conn_type[strcspn(conn_type, "\n")] = 0;

    if (strcmp(conn_type, CONNECTION_TYPE) != 0) {
        fputs("didn't receive a connection from otp_enc\n", stderr);
        write(socket, CONNECTION_BAD, strlen(CONNECTION_BAD)+1);
        return;
    }
    write(socket, CONNECTION_GOOD, strlen(CONNECTION_GOOD)+1);
#ifdef DEBUG
    puts("valid connection");
#endif

    // get plaintext from socket
    char *plaintext = NULL;
    char *key = NULL;

    // jank debugging time
    // char *asdf = malloc(5000);
    // read(socket, asdf, 5000);
    // read(socket, asdf, 5000);


    recv_data(socket, &plaintext, &key);
#ifdef DEBUG
    printf("plaintext: %s\nkey: %s\n", plaintext, key);
#endif

    // send ciphertext
    send_ciphertext(socket, plaintext, key);

    // free
    if (plaintext != NULL) free(plaintext);
    if (key != NULL) free(key);
}

// get the plaintext and key from the client, returns total bytes received or -1
// NOTE: the client must pad the plaintext and key so that the length for each % BUFFER_BASE = 0
int recv_data(int socket, char **plaintext_ptr, char **key_ptr) {
    char *plaintext = *plaintext_ptr;
    char *key = *key_ptr;

    // alloc base memory for plaintext
    plaintext = malloc(sizeof(char) * BUFFER_BASE);
    memset(plaintext, 0, BUFFER_BASE);

    // get data from the socket
    ssize_t bytes_read = read(socket, plaintext, BUFFER_BASE);
    if (bytes_read < 0) {
        fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
        return -1;
    }
#ifdef DEBUG
    else {
        puts(plaintext);
        //printf("got %zu bytes\n", bytes_read);
    }
#endif

    // check if we got the end of the data
    int base_count = 1;
    char *new;
    while (strpbrk(plaintext, "\n") == NULL) {
        // there is more data to be read
        
        // copy over old plaintext to new buffer
        new = malloc(sizeof(char) * (BUFFER_BASE * (++base_count)));
        memset(new, 0, BUFFER_BASE*base_count);
        strcpy(new, plaintext);
        free(plaintext);
        plaintext = new;

        // read in more data
        bytes_read = read(socket, plaintext+(BUFFER_BASE*(base_count-1)), BUFFER_BASE);
        if (bytes_read < 0) {
            fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
            return -1;
        }
#ifdef DEBUG
        else {
            puts(plaintext);
            //printf("got %zu bytes\n", bytes_read);
        }
#endif
    }

    // strip out newline (not a valid character for encryption)
    plaintext[strcspn(plaintext, "\n")] = 0;
#ifdef DEBUG
    printf("got plaintext from client: %s\n", plaintext);
#endif
    
    // alloc base memory for key
    key = malloc(sizeof(char) * BUFFER_BASE);
    memset(key, 0, BUFFER_BASE);

    // get data from the socket
    bytes_read = read(socket, key, BUFFER_BASE);
    if (bytes_read < 0) {
        fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
        return -1;
    }
#ifdef DEBUG
    else {
        printf("got %zu bytes\n", bytes_read);
        puts(key);
    }
#endif

    // check if we got the end of the data
    base_count = 1;
    while (strpbrk(key, "\n") == NULL) {
        // there is more data to be read
        
        // copy over old plaintext to new buffer
        new = malloc(sizeof(char) * (BUFFER_BASE * (++base_count)));
        memset(new, 0, BUFFER_BASE*base_count);
        strcpy(new, key);
        free(key);
        key = new;

        // read in more data
        bytes_read = read(socket, key+(BUFFER_BASE*(base_count-1)), BUFFER_BASE-1);
        if (bytes_read < 0) {
            fprintf(stderr, "error reading from socket: %s\n", strerror(errno));
            return -1;
        }
#ifdef DEBUG
        else {
            // printf("got %zu bytes\n", bytes_read);
            // puts(key);
        }
#endif
    }

    // strip out newline (not a valid character for encryption)
    key[strcspn(key, "\n")] = 0;
#ifdef DEBUG
    printf("got key from client: %s\n", key);
#endif

    *plaintext_ptr = plaintext;
    *key_ptr = key;
    return strlen(plaintext) + strlen(key);
}

// generate and send ciphertext back to client
void send_ciphertext(int socket, const char *plaintext, const char *key) {
    // alloc memory for ciphertext
    int ciphertext_buffer_len = strlen(plaintext) + (strlen(plaintext) % BUFFER_BASE) + 2;
    char *ciphertext = malloc(sizeof(char) * ciphertext_buffer_len);
    memset(ciphertext, 0, ciphertext_buffer_len);

    // loop through the plaintext and do the encryption
    int i, p, k, c;
    for (i = 0; i < strlen(plaintext); i++) {
        p = get_int(plaintext[i]);
        k = get_int(key[i]);

        c = (p - k + 27) % 27;
        ciphertext[i] = get_char(c);
    }
    ciphertext[i] = '\n';

    // send the ciphertext to the client
    // intentionally not using ciphertext_buffer_len
    for (i = 0; i < strlen(ciphertext); i += BUFFER_BASE) {
        if (write(socket, ciphertext+i, BUFFER_BASE) < 0) {
            fprintf(stderr, "error writing to socket: %s\n", strerror(errno));
        }
    }

    // free
    free(ciphertext);
}

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