#include "signals.h"

void register_child() {
    // register child sigint handler
    struct sigaction sa_sigint;
    sa_sigint.sa_handler = handle_sigint_child;
    sigfillset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = 0;
    sigaction(SIGINT, &sa_sigint, NULL);

    // register child sigtstp handler
}

void handle_sigint_child(int signo) {
#ifdef DEBUG
    puts("got sigint (child)");
#endif

    exit(0);
}


void register_parent() {
    // register parent sigint handler
    struct sigaction sa_sigint;
    sa_sigint.sa_handler = handle_sigint_parent;
    sigfillset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = 0;
    sigaction(SIGINT, &sa_sigint, NULL);
    
    // register sigtstp handler
    struct sigaction sa_sigtstp;
    sa_sigtstp.sa_handler = handle_sigtstp;
    sigfillset(&sa_sigtstp.sa_mask);
    sa_sigtstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_sigtstp, NULL);
    foreground_mode = false;
}

void handle_sigint_parent(int signo) {
#ifdef DEBUG
    puts("got sigint (parent)");
#endif

    // do nothing
    puts("");
}

void handle_sigtstp(int signo) {
#ifdef DEBUG
    puts("got sigtstp");
#endif

    foreground_mode = !foreground_mode;

    puts("");
    puts(foreground_mode ? "Entering foreground-only mode (& is now ignored)" : "Exiting foreground-only mode");
}