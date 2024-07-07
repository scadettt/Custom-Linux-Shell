#include "system_program.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void execute() {
    char buffer[1024];
    int daemon_count = 0;

    // popen executes the ps function and returns a file pointer that we can use to access the output of ps
    FILE *file_content = popen("ps -efj | grep dspawn | grep -Ev 'tty|pts'", "r");
    if (file_content == NULL) {
        printf("Error: empty file_content");
        exit(EXIT_FAILURE);
    } else {
        // From the notes, we know that each line entry in the file corresponds to each new live daemon
        // So we can count number of entries which also counts the number of live daemons
        while (fgets(buffer, sizeof(buffer), file_content) != NULL) {
            daemon_count++;
        }
    }
    pclose(file_content);

    if (daemon_count == 0) {
        printf("No daemon\n");
    } else {
        printf("Live daemon counts: %d\n", daemon_count);
    }
}

int main() {
    execute();
    return 0;
}
