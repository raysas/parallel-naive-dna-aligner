#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// # -- threads vs processes

int main(int argc, char* argv[]){
    int x=2;
    int pid=fork();

    if (pid==-1){
        return 1;
    }
    if (pid==0){
        x++; //gets increamented on one process only
        // threads will share memory (if incremented in one thread, will be seen in the 2nd
        // in porcess no
    }
    sleep(2);
    printf("x is %d\n",x);

    printf("process id %d\n",getpid());
    if (pid!=0){
        wait(NULL);
    }
    return 0;

    return 0;
}