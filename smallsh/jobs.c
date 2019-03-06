#include "jobs.h"

void jobs_init() {
    // alloc/init BackgroundProcs struct to track background procs
    procs = malloc(sizeof(BackgroundProcs));
    procs->num = 5;
    procs->pids = malloc(sizeof(pid_t) * procs->num);
    for (int i = 0; i < procs->num; i++) procs->pids[i] = -1;

    // initalize mutex
    pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
    proc_lock = a;
    pthread_mutex_unlock(&proc_lock);

    // start thread
    pthread_create(&bkgd_thread, NULL, check_bkgds, NULL);

    // register exit handler
    atexit(&kill_bkgds);
}

// check for any background processes if they finished
// this should run in it's own thread
void* check_bkgds(void *_) {
    int check, ret;
    while (true) {
        for (int i = 0; i < procs->num; i++) {
            if (procs->pids[i] > 0) {
                // get proc status
                ret = waitpid(procs->pids[i], &check, WNOHANG);

                // check if it exited or got signaled to exit
                if (ret > 0) {
                    printf("\nbackground pid %d is done: ", procs->pids[i]);
                    if (WIFEXITED(check)) {
                        printf("exit value %d\n", WEXITSTATUS(check));
                    } else if (WIFSIGNALED(check)) {
                        printf("terminated by signal %d\n", WTERMSIG(check));

                    }

                    // remove pid from array
                    pthread_mutex_lock(&proc_lock);
                    procs->pids[i] = -1;
                    pthread_mutex_unlock(&proc_lock);
                    printf(": ");
                    fflush(stdout);
                }
            }
        }
        my_sleep(100);
    }
}

// sleep for $ms milliseconds
void my_sleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// register a background process
void add_proc(pid_t pid) {
    // try to add the pid to an existing array slot
    for (int i = 0; i < procs->num; i++) {
        if (procs->pids[i] < 0) {
            // found an empty slot
            pthread_mutex_lock(&proc_lock);
            procs->pids[i] = pid;
            pthread_mutex_unlock(&proc_lock);
            return;
        }
    }

    // couldn't find a slot
    // make new array
    pid_t *new_pids = malloc(sizeof(pid_t) * procs->num*2);
    for (int i = procs->num; i < procs->num*2; i++) new_pids[i] = -1;

    // copy over old pids
    pthread_mutex_lock(&proc_lock);
    for (int i = 0; i < procs->num; i++) new_pids[i] = procs->pids[i];

    // set new pid
    new_pids[procs->num] = pid;

    // free old array
    free(procs->pids);

    // save new array
    procs->pids = new_pids;
    pthread_mutex_unlock(&proc_lock);
}

// kill all background jobs and free memory
// register with atexit()
void kill_bkgds() {
    // kill thread
    pthread_cancel(bkgd_thread);

    // kill background procs
    for (int i = 0; i < procs->num; i++) {
        if (procs->pids[i] > 0) {
            kill(procs->pids[i], SIGKILL);
        }
    }

    // free memory
    free(procs->pids);
    free(procs);
}