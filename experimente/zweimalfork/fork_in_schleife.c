
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

int main() {

    for(int i = 0; i<= 3; i++) {
        pid_t child_pid = fork();
        if( child_pid == 0) {
            printf(" my rank ist %d", i);
            printf(" Kindprozess: my pid ist = %d\n", getpid());
            printf(" Kindprozess: pid of my Elternprozess is = %d\n\n", getppid());
            printf("\n");
        }
        else {
            //printf(" Elternprozess: my pid ist = %d\n", getpid());
            //printf(" Der Kindprozess %d wurde erzeugt\n\n", child_pid);
            sleep(1);
        }
    }

}
