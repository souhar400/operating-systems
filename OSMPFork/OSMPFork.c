//
// Created by lukas on 01.04.22.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FORKERR 10
#define EXECERR 11
#define WAITERR 12

int main(){
    pid_t pid, wpid;
    int wstatus;
    char *envp[] = {"PATH=/home/ms/l/lw874718/CLionProjects/grp28/lib", NULL};
    char *argp[] = {"execv*", "tests", NULL};
    for(int i = 0; i < 6; i++) {
        pid = fork();
        if (pid == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(FORKERR);
        }

        if (pid == 0) {
            switch (i) {
                case 0:
                    execl("/home/ms/l/lw874718/CLionProjects/grp28/lib/Echoall",
                          "execl", "test1", "letztes Argument", NULL);
                    break;
                case 1:
                    execlp("../lib/Echoall", "execlp", "test2", NULL);
                    break;
                case 2:
                    execle("Echoall","execle", "test3", NULL, envp);
                    break;
                case 3:
                    execv("/home/ms/l/lw874718/CLionProjects/grp28/lib/Echoall", argp);
                    break;
                case 4:
                    execve("Echoall", argp, envp);
                    break;
                case 5:
                    execvp("../lib/Echoall", argp);
                    break;
                default:
                    break;
            }
            fprintf(stderr, "%s\n", strerror(errno));
            exit(EXECERR);
        }

    }
    //wpid = waitpid(-1, &wstatus, 0);
    while((wpid = wait(&wstatus)) > 0) {
        if (wpid == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(WAITERR);
        }
        printf("Child: %d exited with status: %d\n", wpid, WEXITSTATUS(wstatus));
    }

    return 0;
}