#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int x=2;



void* routine(){
    x++;
    sleep(2);
    printf("x is %d\n",x);
    
}

void* routine2(){
    sleep(2);
    // printf("preocess if %d\n",getpid());
    // will have 2 of same id => multiple threads on the same process
    // also in terms of address space 
    printf("x is %d\n",x);
}

int main(int argc, char* argv[]){
    pthread_t t1, t2;

    if (pthread_create(&t1,NULL,&routine,NULL)){
        return 1;
    }
    if (pthread_create(&t2,NULL,&routine2,NULL)){
        return 2;
    }
    if (pthread_join(t1,NULL)){
        return 3;
    }
    if (pthread_join(t2,NULL)){
        return 4;
    }
    return 0;
}