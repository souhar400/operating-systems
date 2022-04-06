//
// Created by studi on 05.04.22.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FORKERR 10

int main() {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FORKERR);
    } else if (pid == 0) {
        sleep(1);
        printf(" Im KindProcess\n");
        printf(" Kindprozess: my pid ist = %d\n", getpid());
        printf(" Kindprozess: ich muss für 5 Sekunden schlafen. Elternprozess wird auf mich warten\n");
        sleep(5);
        }
    else{
        printf("Im Elternprozess\n");
        printf("Elternprozess: my pid ist = %d\n", getpid());
        printf("Elternprozess: ich warte auf Kindprozess bis er fertig ist\n");
        sleep(1);
        waitpid(pid, NULL, 0);
        //wait(NULL);
        printf("\nElternprozess: ich bin wieder da\n");
    }
}
