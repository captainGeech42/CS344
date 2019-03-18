#include "otp_enc.h"

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
    char *ciphertext, *key;
    int ciphertext_size = get_file(argv[1], &ciphertext);
    int key_size = get_file(argv[2], &key);

    // make sure we have a valid key length
    if (strlen(key) < strlen(ciphertext)) {
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
    if (!send_data(socket_fd, ciphertext, ciphertext_size)) {
        fputs("Failed to send ciphertext\n", stderr);
        return 1;
    }
    if (!send_data(socket_fd, key, key_size)) {
        fputs("Failed to send key\n", stderr);
        return 1;
    }

    // get plaintext from server
    char *plaintext;
    if (recv_data(socket_fd, &plaintext) < 0) {
        fputs("Failed to recieve plaintext\n", stderr);
        return 1;
    }

    // print plaintext
    printf("%s", plaintext);

    // cleanup
    free(plaintext);
    close(socket_fd);
}
