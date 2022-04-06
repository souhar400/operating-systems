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
    int my_var= 10;
    if (pid == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FORKERR);
    } else if (pid == 0) {
        printf(" Im KindProcess\n");
        printf(" Kindprozess: my pid ist = %d\n", getpid());
        for(int i =1; i<= 5; i++ ){
            my_var--;
            printf( " Kindprozess -> Ausgabe: %d\n", my_var);
            sleep(1);
        }
    }

    else{
        printf("Im Elternprozess\n");
        printf("Elternprozess: my pid ist = %d\n", getpid());
        for(int i =1; i<= 5; i++ ){
            my_var++;
            printf( "Elternprozess -> Ausgabe: %d\n", my_var);
            sleep(1);
        }
    }
}
