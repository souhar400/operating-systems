//
// Created by studi on 06.04.22.
//

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FORKERR 10

static int global_var = 1;
int main (void) {
    pid_t pid;
    int lokal_var = 1;
    switch (pid = fork()) {
        case -1:
            fprintf(stderr, "%s\n", strerror(errno));
            exit(FORKERR);
            break;
        case 0:
            sleep(1);   /* Kurze Pause */
            printf ("--- Im Kindprozess ---\n");
            printf ("global_var = %d Speicheradresse : %p\n", global_var, &global_var);
            printf ("lokal_var  = %d Speicheradresse : %p\n", lokal_var, &lokal_var);
            ++global_var;
            ++lokal_var;
            printf ("--- Im Kindprozess ---\n");
            printf ("global_var = %d Speicheradresse : %p\n", global_var, &global_var);
            printf ("lokal_var  = %d Speicheradresse : %p\n", lokal_var, &lokal_var);
            break;
        default:
            printf ("--- Im Elternprozess ---\n");
            printf ("global_var = %d Speicheradresse : %p\n", global_var, &global_var);
            printf ("lokal_var  = %d Speicheradresse : %p\n", lokal_var, &lokal_var);
            sleep (2);
            printf ("--- Im Elternprozess ---\n");
            printf ("global_var = %d Speicheradresse : %p\n", global_var, &global_var);
            printf ("lokal_var  = %d Speicheradresse : %p\n", lokal_var, &lokal_var);
            break;
    }
    return EXIT_SUCCESS;
}