#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


int primes[10]={2,3,5,7,11,13,17,19,23,29};

void* print_prime(void* arg){
    sleep(1);
    int index=*(int*)arg;
    printf("this is our prime %d\n", primes[index]);
}

int main(int argc, char* argvp[]){
    pthread_t th[10];
    int i;
    for (i=0;i<10;i++){
        if(pthread_create(&th[i],NULL,&print_prime,&i)!=0){
            perror("failed to creat thread\n");
        }
    }
    for (i=0;i<10;i++){
        if(pthread_join(th[i],NULL)!=0){
            return 2;
        }
    }
}