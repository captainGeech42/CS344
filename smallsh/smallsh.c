#include "smallsh.h"

// main
int main() {
    register_parent();
    jobs_init();

    while (!fin) {
        // alloc/init command
        Command *cmd = alloc_cmd();

        // get command string
        prompt(cmd);

        // parse pieces
        if (!parse(cmd)) goto end_loop;

#ifdef DEBUG
        printf("program: %s\n", cmd->program);
        printf("num args: %d\n", cmd->argc);
        for (int i = 0; i < cmd->argc; i++) printf("argv[%d] = %s\n", i, cmd->argv[i]);
        printf("input file: %s\n", cmd->input_file);
        printf("output file: %s\n", cmd->output_file);
        printf("background: %s\n", cmd->background ? "true" : "false");
#endif

        run(cmd);

end_loop:
        // free cmd
        free_cmd(cmd);
    }

    return 0;
}

// allocate memory to hold relevant command information
// returns a pointer to the struct
Command* alloc_cmd() {
    Command *cmd = malloc(sizeof(Command));

    cmd->line = malloc(sizeof(char) * MAX_COMMAND_LINE);
    memset(cmd->line, 0, MAX_COMMAND_LINE);

    cmd->program = malloc(sizeof(char) * MAX_COMMAND_LINE);
    memset(cmd->program, 0, MAX_COMMAND_LINE);

    cmd->input_file = malloc(sizeof(char) * MAX_COMMAND_LINE);
    memset(cmd->input_file, 0, MAX_COMMAND_LINE);

    cmd->output_file = malloc(sizeof(char) * MAX_COMMAND_LINE);
    memset(cmd->output_file, 0, MAX_COMMAND_LINE);
    
    cmd->argv = malloc(sizeof(char *) * MAX_NUM_ARGS);
    for (int i = 0; i < MAX_NUM_ARGS; i++) {
        cmd->argv[i] = malloc(sizeof(char) * MAX_COMMAND_LINE);
        memset(cmd->argv[i], 0, MAX_COMMAND_LINE);
    }

    cmd->argc = 0;
    cmd->background = false;

    return cmd;
}

// free memory for the command struct
void free_cmd(Command *cmd) {
    for (int i = 0; i < MAX_NUM_ARGS; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
    
    free(cmd->line);
    free(cmd->program);
    free(cmd->input_file);
    free(cmd->output_file);

    free(cmd);
    cmd = NULL;
}

// prints the prompt, gets command string from the user
void prompt(Command *cmd) {
    printf(": ");

    fgets(cmd->line, MAX_COMMAND_LINE, stdin);
    cmd->line[strcspn(cmd->line, "\n")] = 0;
}

// parse out important pieces
bool parse(Command *cmd) {
    char *tmp;
    bool mallocd = false;

    // check for blank or comment lines
    if (strlen(cmd->line) == 0 || *(cmd->line) == '#') return false;

    // get program
    tmp = strtok(cmd->line, " ");
    strcpy(cmd->program, tmp);

    // program has to be argv[0]
    strcpy(cmd->argv[cmd->argc++], cmd->program);

    // get args/redirects/background
    while ((tmp = strtok(NULL, " ")) != NULL) {
        if (strstr(tmp, "$$") != NULL) {
            // we need to expand $$ to smallsh's pid
            
            // get pid as string
            char pid[6]; // max pid is 65535, plus \x00
            memset(pid, 0, 6);
            sprintf(pid, "%d", getpid());

            // alloc new memory
            char *new_tmp = malloc(sizeof(char) * (strlen(tmp) + strlen(pid))); // a few extra bytes, eh
            memset(new_tmp, 0, strlen(tmp) + strlen(pid));
            if (new_tmp == NULL) {
                fputs("couldn't malloc for $$\n", stderr);
                return false;
            }

            // move over first part of tmp
            strncpy(new_tmp, tmp, (size_t)(strstr(tmp, "$$") - tmp));

            // move over pid
            strcat(new_tmp, pid);

            // move over rest of string
            strcat(new_tmp, strstr(tmp, "$$") + 2);

            // move it back to tmp;
            tmp = new_tmp;
            mallocd = true;
        }

        if (tmp[0] == '<') {
            // there is a < in our token (input redirect)

            if (strlen(tmp) > 1) {
                // there wasn't a space between the < and the filename
                // remove the <
                memmove(tmp, tmp+1, strlen(tmp));
            } else {
                // there was a space
                // get the next token, it's the filename
                tmp = strtok(NULL, " ");
            }

            // make sure it's valid
            if (tmp == NULL || strpbrk(tmp, "<>&") != NULL) {
                fputs("invalid command (check redirects)\n", stderr);
                return false;
            }

            // save the filename to the struct
            strcpy(cmd->input_file, tmp);
        } else if (tmp[0] == '>') {
            // there is a > in our token (output redirect)

            if (strlen(tmp) > 1) {
                // there wasn't a space between the > and the filename
                // remove the >
                memmove(tmp, tmp+1, strlen(tmp));
            } else {
                // there was a space
                // get the next token, it's the filename
                tmp = strtok(NULL, " ");
            }

            // make sure it's valid
            if (tmp == NULL || strpbrk(tmp, "<>&") != NULL) {
                fputs("invalid command (check redirects)\n", stderr);
                return false;
            }

            // save the filename to the struct
            strcpy(cmd->output_file, tmp);
        } else if (strcmp("&", tmp) == 0) {
            // we have an & (background proccess)
            cmd->background = true;
        } else if (strpbrk(tmp, "<>&") != NULL) {
            // we have a special character that wasn't already handled
            // meaning, it shouldn't be there
            fputs("invalid character in arguments list\n", stderr);
            return false;
        }
        else {
            // we have an argument
            strcpy(cmd->argv[cmd->argc++], tmp);
        }

        // check if we malloc'd
        if (mallocd) free(tmp);
    }

    // we didn't get any errors parsing
    return true;
}

// run the command
void run(Command *cmd) {
    int ret;
    if (strcmp(cmd->program, "cd") == 0) {
        // run built-in cd
        my_cd(cmd->argv[1]);
    } else if (strcmp(cmd->program, "exit") == 0) {
        // run built-in exit
        my_exit();
        fin = true;
        return;
    } else if (strcmp(cmd->program, "status") == 0) {
        // run built-in status
        my_status(proc_status);
    } else {
        // not a built-in
        pid_t pid = fork();
        switch (pid) {
            case -1:
                // something went wrong
                fprintf(stderr, "error when forking: %s\n", strerror(errno)); fflush(stderr);
                exit(1);
                break;
            case 0:
                // child process

                // handle input/output redirects
                if (strlen(cmd->input_file) > 0) {
                    // dup2 stdin
                    int input = open(cmd->input_file, O_RDONLY);
                    ret = dup2(input, 0);

                    if (ret == -1) {
                        fprintf(stderr, "error on input dup2: %s\n", strerror(errno)); fflush(stderr);
                        exit(2);
                    }
                } else if (cmd->background) {
                    // dup2 stdin to null
                    int input = open("/dev/null", O_RDONLY);
                    ret = dup2(input, 0);

                    if (ret == -1) {
                        fprintf(stderr, "error on input dup2: %s\n", strerror(errno)); fflush(stderr);
                        exit(2);
                    }
                }

                if (strlen(cmd->output_file) > 0) {
                    // dup2 stdout
                    int output = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    ret = dup2(output, 1);

                    if (ret == -1) {
                        fprintf(stderr, "error on output dup2: %s\n", strerror(errno)); fflush(stderr);
                        exit(3);
                    }
                } else if (cmd->background) {
                    // dup2 stdout to null
                    int output = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    ret = dup2(output, 1);

                    if (ret == -1) {
                        fprintf(stderr, "error on output dup2: %s\n", strerror(errno)); fflush(stderr);
                        exit(3);
                    }
                }

                // copy over arguments needed (+1 for null terminator)
                char **argv = malloc(sizeof(char *) * (cmd->argc + 1));
                for (int i = 0; i < cmd->argc; i++) {
                    argv[i] = malloc(sizeof(char) * (strlen(cmd->argv[i]) + 1));
                    strcpy(argv[i], cmd->argv[i]);
                }

                // terminate argv array
                argv[cmd->argc] = NULL;

                // register signal handlers
                register_child();

                // spawn process
                ret = execvp(cmd->program, argv);

                if (ret == -1) {
                    fprintf(stderr, "error on execvp: %s\n", strerror(errno)); fflush(stderr);
                    exit(1);
                }
                break;
            default:
                // parent process

                // check if we should be in background or foreground
                if (cmd->background && !foreground_mode) {
                    // run in background

                    // save the pid
                    add_proc(pid);

                    // print the pid
                    printf("background pid is %d\n", pid);
                } else {
                    // run in foreground, wait for execution to finish

                    waitpid(pid, &proc_status, 0);
                    proc_status = WEXITSTATUS(proc_status);
                }
                break;
        }

    }
}