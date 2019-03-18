#include "otp_enc_d.h"

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

    int listener_socket = start_server(port);

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

void process(int socket_fd) {
    // handshake with client
    if (!handshake_server(socket_fd)) {
        fputs("client failed handshake\n", stderr);
        exit(2);
    }

    // get data from client
    char *plaintext, *key;
    if (recv_data(socket_fd, &plaintext) < 0) {
        fputs("error getting plaintext from client\n", stderr);
        exit(1);
    }
#ifdef DEBUG
    else {
        printf("got plaintext from server: %s\n", plaintext);
    }
#endif
    if (recv_data(socket_fd, &key) < 0) {
        fputs("error getting key from client\n", stderr);
        exit(1);
    }
#ifdef DEBUG
    else {
        printf("got key from server: %s\n", key);
    }
#endif

    // encode data
    encode(plaintext, key);

    // send ciphertext back
    if (!send_data(socket_fd, plaintext, strlen(plaintext))) {
        fputs("error sending ciphertext to client\n", stderr);
        exit(1);
    }

    free(plaintext);
    free(key);
}

// generate ciphertext (saved in plaintext)
void encode(char *plaintext, const char *key) {
    // loop through the plaintext and do the encryption
    int i, p, k, c;
    for (i = 0; i < strlen(plaintext)-1; i++) {
        p = get_int(plaintext[i]);
        k = get_int(key[i]);

        c = (p + k) % 27;
        plaintext[i] = get_char(c);
    }
    plaintext[i] = '\n';
    plaintext[i+1] = '\0';
}