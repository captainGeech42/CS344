#include "builtins.h"

void my_exit() {
    //TODO kill background procs

    // don't actually exit here, we have to clean up memory in main
}

void my_cd(const char *dir) {
    int ret;
    if (dir == NULL || strlen(dir) == 0) {
        // no directory specified;, go to home dir
        ret = chdir(getenv("HOME"));
    } else {
        // directory specified, go there
        ret = chdir(dir);
    }

    if (ret != 0) {
        // there was an error
        fprintf(stderr, "error: %s\n", strerror(errno));
    }
}

void my_status(int status) {
    printf("exit value %d\n", status);
}