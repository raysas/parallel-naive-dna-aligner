#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int mails=0;

void* routine(){
    for (int i=0;i<1000000;i++){
        mails++;
        // read mails
        // inccreament
        // write it back to memory -> write mails
        // -- its fine if on 1 thread
        // -- on multiple threads doesnt work here
        // -- RACE CONDITION
    }
}

int main(int agrc, char* argv[]){

    pthread_t t1;
    pthread_t t2;
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

    printf("number of mails: %d\n",mails);
    return 0;
}