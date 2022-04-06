//
// Created by studi on 01.04.22.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FORKERR 10
#define EXECERR 11

int main() {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FORKERR);
    } else if (pid == 0) {
        printf(" Im KindProcess\n");
        execl("/home/studi/ClionProjects/grp28/bin/Echoall", "Echoall", "Argt1", "Argt2", NULL);
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXECERR);
    }

    else{
        printf("Im Elternprozess\n");
        printf("Elternprozess: my pid ist = %d\n", getpid());
        sleep(1);
    }
}
