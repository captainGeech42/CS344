#include "smallsh.h"

// main
int main() {
    while (!fin) {
        // alloc/init command
        Command *cmd = alloc_cmd();

        // get command string
        prompt(cmd);

        // parse pieces
        if (!parse(cmd)) continue;

#ifdef DEBUG
        printf("program: %s\n", cmd->program);
        printf("num args: %d\n", cmd->argc);
        for (int i = 0; i < cmd->argc; i++) printf("argv[%d] = %s\n", i, cmd->argv[i]);
        printf("input file: %s\n", cmd->input_file);
        printf("output file: %s\n", cmd->output_file);
        printf("background: %s\n", cmd->background ? "true" : "false");
#endif

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

    // get program
    tmp = strtok(cmd->line, " ");
    strcpy(cmd->program, tmp);

    // get args/redirects/background
    while ((tmp = strtok(NULL, " ")) != NULL) {
        if (strstr(tmp, "<") != NULL) {
            // there is a < in our token (input redirect)

            if (strlen(tmp) > 1) {
                // there wasn't a space between the < and the filename
                // remove the <
                memmove(tmp, tmp+1, strlen(tmp));
            } else {
                // there was a space
                // get the next token, it's the filename
                tmp = strtok(NULL, " ");

                // make sure it's valid
                if (tmp == NULL || strspn(tmp, "<>&") != 0) {
                    fputs("invalid command (check redirects)", stderr);
                    return false;
                }
            }

            // save the filename to the struct
            strcpy(cmd->input_file, tmp);
        } else if (strstr(tmp, ">") != NULL) {
            // there is a > in our token (output redirect)

            if (strlen(tmp) > 1) {
                // there wasn't a space between the > and the filename
                // remove the >
                memmove(tmp, tmp+1, strlen(tmp));
            } else {
                // there was a space
                // get the next token, it's the filename
                tmp = strtok(NULL, " ");

                // make sure it's valid
                if (tmp == NULL || strspn(tmp, "<>&") != 0) {
                    fputs("invalid command (check redirects)", stderr);
                    return false;
                }
            }

            // save the filename to the struct
            strcpy(cmd->output_file, tmp);
        } else if (strcmp("&", tmp) == 0) {
            // we have an & (background proccess)
            cmd->background = true;
        } else {
            // we have an argument
            strcpy(cmd->argv[cmd->argc++], tmp);
        }
    }

    // we didn't get any errors parsing
    return true;
}