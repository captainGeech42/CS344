#include "otp_enc.h"

int main(int argc, char **argv) {
    // validate arguments
    if (argc != 4) {
        fprintf(stderr, "usage: %s [plaintext] [key] [port]\n", argv[0]);
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

    // connect to server
    int socket_fd = connect_to_server(port);

    // handshake
    if (!handshake_client(socket_fd)) {
        fputs("Invalid handshake with server (wrong port?)\n", stderr);
        return 2;
    }

    // send data
    if (!send_data(socket_fd, plaintext, plaintext_size)) {
        fputs("Failed to send plaintext\n", stderr);
        return 1;
    }
    if (!send_data(socket_fd, key, key_size)) {
        fputs("Failed to send key\n", stderr);
        return 1;
    }

    // get ciphertext from server
    char *ciphertext;
    if (recv_data(socket_fd, &ciphertext) < 0) {
        fputs("Failed to recieve ciphertext\n", stderr);
        return 1;
    }

    // print ciphertext
    printf("%s", ciphertext);

    // cleanup
    free(ciphertext);
    close(socket_fd);
}
