# OpenMP

*OpenMP parallel programming using omp.h library in C*

**What is OpenMP?**  
OpenMP stands for Open Multi-Processing, an API supporting multi-platform shared memory multiprocessing programming in C, C++, and Fortran. Unlike posix  it's designed to be simple and flexible, allowing developers to easily parallelize their code by adding compiler directives (`pragmas`) to indicate which parts of the code should be executed in parallel.  
It's called a fork-join model because the program starts with a single thread (the master thread), which then forks additional threads to execute parallel regions of code. Once the parallel region is complete, the threads join back into the master thread.

It's relatively easy to sue, the importance lies in understanding shared vs private variables, and how to manage synchronization between threads when accessing shared resources.

**How to use in C**  
Need to include the library at the beginning of the code:
```c
#include <omp.h>
```
Using it really boils down to adding `#pragma omp` directives before loops or sections of code that you want to parallelize, one main directive that should be present to indicate parallel regions is:
```c
#pragma omp parallel{
    // -- code to be executed in parallel
}
```
For compilation, need to add the `-fopenmp` flag to gcc:
```bash
gcc -fopenmp -o my_program my_program.c
```

| script | description |
|--------|-------------|
| [`max_random_reduction.c`](../tests/openmp/max_random_reduction.c) |  using OpenMP to find the maximum of random numbers generated in parallel using reduction clause |
| [`max_random_critical.c`](../tests/openmp/max_random_critical.c) | using OpenMP to find the maximum of random numbers generated in parallel using critical section for synchronization |

## Coding OpenMP

### parallel zone

Everything that is meant to be ran in parallel should be enclosed in a parallel zone:
```c
#pragma omp parallel {
    // -- code to be executed in parallel
}
```

It's possible to combine multiple directives in the same pragma, e.g., to create a parallel for loop:
```c
#pragma omp parallel for
```
To define spead sections among threads:
```c
#pragma omp parallel sections
```

### OpenMP threads

By default, OpenMP creates as many threads as there are available CPU cores. However, you can specify the number of threads, access it, and check maximal threads

```c
omp_set_num_threads(2);

printf("num threads: %d\n", omp_get_num_threads());
printf("max threads: %d\n", omp_get_max_threads());
```

Can also set the num of threads directly from the directive:
```c
#pragma omp parallel num_threads(2) {
    // -- code to be executed in parallel
}
```
will suppose number of threads=2 for the rest of this doc, unless specified otherwise

### parallel for

To parallelize a for loop, simply add the `#pragma omp parallel for` directive before the loop:
```c
#pragma omp parallel for
for(int i=0; i<N; i++){
    // -- code to be executed in parallel for each i
}
```

In this case, N will be divided to 2 threads (as specified before), each thread will work on its chunk of iterations. If N=10, 2 threads will iterate over 5 iterations each.

> [!IMPORTANT]
> the end of a for section has an implicit barrier, meaning that all threads wait for each other at the end of the sections construct before proceeding (synchronization) UNLESS the `nowait` clause is used


### private and shared variables

OpenMP is known for its shared memory model, i.e., all threads share the same memory space. However, variables can be either shared or private.


By default, variables declared outside the parallel region are shared among all threads, while variables declared inside the parallel region are private to each thread 
One exception is the loop variable in a parallel for loop, which is private by default.

Behaviors:
- Shared variables: all threads can read and write to the same variable, changes made by one thread are visible to all other threads
- Private variables: each thread has its own copy of the variable, changes made by one thread do not affect the copies of other threads

*what are automatic variables?*  
variables declared inside a function (not static or global) are automatic variables, they are stored in the stack and have a lifetime limited to the function scope. Each thread has its own stack, so automatic variables declared inside a parallel region are private to each thread, these are also not shared among threads.

```c
int outside; // shared variable
int i; // private variable by default in parallel for
#pragma omp parallel {
    int inside; // private variable

    #pragma omp for
    for(i=0; i<N; i++){
        // i is private here    
    }
}
```

### sections

`#pragma omp sections` can have multiple `#pragma omp section` inside it, where sections execute in parallel.

> [!IMPORTANT]
> the end of a section has an implicit barrier, meaning that all threads wait for each other at the end of the sections construct before proceeding (synchronization) UNLESS the `nowait` clause is used

```c
#pragma omp parallel {
    #pragma omp sections
    {
        #pragma omp section
        {
            // -- code for section 1
        }
        #pragma omp section
        {
            // -- code for section 2
        }
    } // implicit barrier here
    // -- code here will be executed after all sections are done
}
```

### master

*One time execution*  
`#pragma omp master`, executed only by the master thread, no implicit synchronization here

### single

*One time execution*  
`#pragma omp single`, executed by a single thread (master or not), no implicit synchonization here 
*(recall what does synchronization mean: all threads wait for each other at a certain point before proceeding, so that they are all at the same stage of execution)*

### critical

`#pragma omp critical`, executed by one thread at a time (not in parallel) but many times not just once like master or single  
equivalent to mutex lock in posix threads

### barrier

`#pragma omp barrier`, synchronization point where all threads wait for each other before proceeding  
Section does not start before all threads reach the barrier

```c
#pragma omp parallel {
    //...
    #pragma omp barrier
    // code here will be executed after all threads reach the barrier
}
```

> There's also lock for synchronization, `omp_lock_t` type and associated functions


```c
omp_lock_t my_lock;
omp_init_lock(&my_lock); 

#pragma omp parallel {
    omp_set_lock(&my_lock);
    // -- critical section
    omp_unset_lock(&my_lock);
}
omp_destroy_lock(&my_lock);
```

### parametrizing directives

Can add clauses to directives to modify their behavior, e.g.,
```c
#pragma omp parallel num_threads(4) private(var1, var2) shared(var3)
// -- example of a sharing variable clause
```

#### IF clause
`if(condition)`, if condition is true, the parallel region is executed in parallel, otherwise it is executed serially:
```c
#pragma omp parallel if(N>1000) 
```

#### REDUCTION clause
`reduction(operator: variable)`, used to perform a reduction operation on a variable across all threads

Possible operators: `+`, `-`, `*`, `&`, `|`, `^`, `&&`, `||`, `max`, `min`

notes:
* variable must be declared outside the parallel region
* each thread has its own private copy of the variable (initialized to the identity value of the operator, e.g., for sum it's 0, for product it's 1)
* at the end of the parallel region, the private copies are combined using the specified operator and stored in the original variable
```c
int sum = 0;
#pragma omp parallel for reduction(+: sum)
for(int i=0; i<N; i++){
    sum += i;
}
```

#### NOWAIT clause

The long talked about implicit barrier at the end of sections and for constructs can be removed using the `nowait` clause, allowing threads to proceed without waiting for each other (asynchrony):
```c
#pragma omp for nowait
for(int i=0; i<N; i++){
    //
}
```


### ORDERED clause

*clause and a directive*  
`#pragma omp ordered`, used to ensure that certain sections of code are executed in the order of the loop iterations, even when the loop is parallelized (sequential part inside a parallel loop)  

```c
#pragma omp parallel for ordered
for(int i=0; i<N; i++){
    // -- parallel code
    #pragma omp ordered
    {
        // -- code here will be executed in order of i
    }
}
```

### SCHEDULE clause

`schedule(type, chunk_size)`, used to specify how iterations of a parallel loop are divided among threads (only for `do` and `for` directives)

types:
- `static`: iterations are divided into chunks of size `chunk_size` and assigned to threads - **STRICTLY EQUAL**
- `dynamic`: iterations are divided into chunks of size `chunk_size`, and threads request new chunks as they finish their current work
- `guided`: similar to dynamic, but chunk sizes decrease over time to reduce overhead (its practically dynamic with decreasing chunk sizes)

