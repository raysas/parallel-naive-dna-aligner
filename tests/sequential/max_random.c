#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_NUMBERS 1000 * 1000 * 1000

int main(){
    unsigned int max=0;
    unsigned int seed;
    unsigned int i;

    seed = (unsigned int)time(NULL); //seed with current time

    for(i=0;i<NUM_NUMBERS;i++){
        unsigned int r=rand_r(&seed);
        // rand_r generates a random number
        // its safer to use than rand in multithreaded programs
        // why? because rand uses a global state
        // rand_r uses a state passed by reference (pointer to a seed variable)
        // returns a pseudo-random number in the range 0 to RAND_MAX (pseudo because its deterministic based on the seed)
        if (r>max){
            max=r;
        }

    }
    printf("max is %u\n",max);
    exit(EXIT_SUCCESS);
}