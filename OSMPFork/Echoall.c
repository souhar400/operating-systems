//
// Created by lukas on 01.04.22.
//

#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv){
    pid_t cpid = getpid();

   for(int i = 0; i < argc; i++){
        printf("%s\n", argv[i]);
    }
    printf("This was from Child process: %d\n\n", cpid);
    return 0;
}