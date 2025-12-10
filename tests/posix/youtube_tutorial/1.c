#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* routine(){
    printf("Test from threads\n");
    sleep(3);
    printf("Exiting routine.\n");
}

int main(int agrc, char* argv[]){

    // # -- where to store some info on the thread:
    pthread_t t1;
    pthread_t t2;

    // -- initializing the thread
    // -- need to check if i can really create these threads
    
    // -- this call will return an int, if not a 0 its an error
    if(pthread_create(&t1,NULL,&routine,NULL)!=0){
        return 1;
    }
    
    if(pthread_create(&t2,NULL,&routine,NULL)!=0){
        return 2;
    }

    if (pthread_join(t1,NULL)!=0){
        return 3;
    }

    if(pthread_join(t2,NULL)!=0){
        return 4;
    }


    return 0;
}