#include "keygen.h"

int main(int argc, char **argv) {
    // validate arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s [key length]\n", argv[0]);
        return 1;
    }

    // make sure a valid length was specified
    int keylen = atoi(argv[1]);
    if (keylen < 1) {
        fputs("invalid key length specified\n", stderr);
        return 1;
    }

    // seed rng
    srand(time(NULL));

    // alloc memory for the key
    char *key = malloc(sizeof(char) * keylen);
    bzero(key, keylen);

    // generate key
    int i,r;
    for (i = 0; i < keylen; i++) {
        r = rand() % 27;
        if (r == 26) {
            // add a space
            key[i] = ' ';
        } else {
            // add a letter
            key[i] = r + 'A';
        }
    }
    key[i] = '\0';

    // print key
    puts(key);

    // cleanup
    free(key);
    return 0;
}