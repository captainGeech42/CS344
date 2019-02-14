#include "smallsh.h"

int main(int argc, char **argv) {
    char *command_line = alloc_cl();

    while (!exit) {
        memset(command_line, 0, MAX_COMMAND_LINE);

        prompt(&command_line);

#ifdef DEBUG
        printf("~ got command: %s\n", command_line);
#endif
    }

    free(command_line);
}

char* alloc_cl() {
    return malloc(sizeof(char) * MAX_COMMAND_LINE);
}

// prints the prompt, gets command string from the user
void prompt(char **command_line) {
    printf(": ");

    fgets(*command_line, MAX_COMMAND_LINE, stdin);
    (*command_line)[strcspn(*command_line, "\n")] = 0;
}