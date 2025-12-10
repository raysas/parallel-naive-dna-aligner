# POSIX threads

*POSIX threads (pthreads) using the pthread.h library in C.*

**What are POSIX threads?**
POSIX threads allow for parallel execution of multiple tasks within a *single process*, sharing same process context and memory space (no need to move data). It's way faster to create a new thread than forking a new process. Synchronization is lighter and context switching is faster.

**How to use in C?**  
1. `need to include the pthread header:
```c
#include <pthread.h>
```
1. Need to compile with flag:
```bash
gcc -o my_program my_program.c -lpthread
# or
gcc -o my_program my_program.c -pthread
```
This will link the pthread library and allow you to use POSIX threads in your program, as for creation, synchronization and all, will talk about them in a bit

| c code | description |
|-------------|-------------|
| [tests/posix/hello_world.c](../tests/posix/hello_world.c) | example of hello world using 10 threads |
| [tests/sequential/max_random.c](../tests/sequential/max_random.c) | example of finding max number in an array of random numbers using sequential approach |
| [tests/posix/max_random.c](../tests/posix/max_random.c) | example of finding max number in an array of random numbers using multiple threads |

## Coding pthreads

In general we perform 2 main functions when dealing with threads, which will be of this form:

| Function | Description |
|----------|-------------|
| `pthread_create(&thread_id, NULL, function_name, (void *)&arg)` | creates a new thread that runs a specified function with given arguments |
| `pthread_join(thread_id, NULL)` | waits for a specified thread to finish its execution |

To create a thread, need to have several steps:

* define function that all threads will execute (of type `void *function_name(void *arg)`)
* define thread input arguments (if any, best to use a struct to pass multiple arguments)
* define thread identifier variable (thread id of type `pthread_t`, better to use an array if multiple threads)
* call to `pthread_create()` function to create the thread
* call to `pthread_join()` function to wait for the thread to finish 

At the end in main, we will have:  
- definition of thread id array
- definition of thread input data array
- loop 1 to create threads
- loop 2 to join threads

### Function prototype

The function that the thread will execute must have the following prototype:
```c
void *thread_function(void *arg);
```
Where thread_function is a void pointer function that takes a void pointer as an argument. Whatever the argument type is, it can be cast to void pointer when passing it to the thread function, and recast back to the original type inside the function.

*e.g.*
```c
void *print_hello(void *arg) {
    int *num = (int *) arg;
    printf("Hello number %d\n", *num);
    pthread_exit(NULL);
}
```

### input data

A good practice is to create a struct to hold all the input data needed by the thread function. This way, can pass a pointer to the struct as the argument to the thread function.
*e.g.*
```c
typedef struct thread_data {
    int thread_id;
    int info;
} thread_data_t;
// we named the struct thread_data_t
```

This would make a good argument to pass to the thread function, passing will be performed later using `pthread_create()`.  
In fact, it will be given as a casted void pointer of &the struct variable (& is to pass the address, since we want to pass a pointer); e.g., `(void *) &input` for input of type thread_data_t

In the thread function itself, will be recasted back to its original type; e.g., `thread_data_t *data = (thread_data_t *) arg;`, still important to dereference the pointer to access the struct members, e.g., `data->thread_id`

e.g.,
```c
// example creating only one thread

void* thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;
    printf("Thread ID: %d, Info: %d\n", data->thread_id, data->info);
    pthread_exit(NULL);
}

pthread_t thread;
pthread_data_t thread_data;
thread_data.thread_id = 1;
thread_data.info = 42;

pthread_create(&thread, NULL, thread_function, (void *)&thread_data);
```


### Thread identifier

To create a thread, need to declare a variable of type `pthread_t` to hold the thread identifier.
We will be having in main an array of pthread_t to account for multiple threads (declare and not initialize, need to pass the variable to creation function)

### main

In the main function will initialize several variables at the start and create loops depending on the number of threads to create and join them later

Varibales to *decalre* in main:  
* array of pthread_t to hold thread ids
* array of thread_data_t to hold input data for each thread
* iterator variable for loops i
* return code variable rc to check for errors
* *optional*, something to hold info like a global max or index or to pass to threads

```c
#define NUM_THREADS 5

int main(int argc, char** argv) {
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data_array[NUM_THREADS];
    int rc;
    int i;
    int max = 0;  // example
}
```

> [!NOTE]
> most of the times, we will be splitting work among threads, so we will need to pass some info to each thread, like start and end indices, or chunk size, or global data like max value to compare against whatsoever, so at one point we would be passing a varibale like `number_per_thread` to each thread which would be like this

```c
int number_per_thread = total_numbers / NUM_THREADS;
```
Here, ideally, we would save a `number_per_thread` variable inside the `thread_data_t` struct to pass it to each thread.  
To account for all cases, we need to make sure no remainder of thsi division is left without being processed, so can follow 2 strategies:
1. pass the remainder to the last thread only, so it would process `number_per_thread + remainder`
2. distribute the remainder among all threads, so first `remainder` threads would process `number_per_thread + 1`, and the rest would process `number_per_thread`

```c
// example of strategy 1
int remainder = total_numbers % NUM_THREADS;

for (i = 0; i < NUM_THREADS; i++) {
    thread_data_array[i].thread_id = i;
    
    if (i == NUM_THREADS - 1) {
        thread_data_array[i].number_per_thread = number_per_thread + remainder;
    } else {
        thread_data_array[i].number_per_thread = number_per_thread;
    }
}
```
```c
// example of strategy 2
int remainder = total_numbers % NUM_THREADS;
for (i = 0; i < NUM_THREADS; i++) {
    thread_data_array[i].thread_id = i;
    
    if (i < remainder) {
        thread_data_array[i].number_per_thread = number_per_thread + 1;
    } else {
        thread_data_array[i].number_per_thread = number_per_thread;
    }
}
```


### Creating threads with pthread_create()

`pthread_create()` takes 4 arguments:
- pointer to thread it
- thread attributes (can be NULL for default attributes)
- pointer to the function to be executed by the thread
- pointer to the argument to be passed to the thread function

*e.g.* creating one thread 🟩:

```c
void* print_hello(void *arg) {
    int *num = (int *) arg;
    printf("Hello number %d\n", *num);
    pthread_exit(NULL);
}

pthread_t thread;
int thread_arg = 1;

int rc = pthread_create(&thread, NULL, print_hello, (void *)&thread_arg);
if (rc) {
    fprintf("Error:unable to create thread, %d\n", rc);
    exit(EXIT_FAILURE);
}
```
*e.g.* creating 2 threads with just an integer argument 🟨:
```c
//same print_hello function as above

pthread_t threads[2]={1,2};
int thread_args[2] = {10, 20};

pthread_create(&threads[0], NULL, print_hello, (void *)&thread_args[0]);
pthread_create(&threads[1], NULL, print_hello, (void *)&thread_args[1]);
```

*e.g.* creating multiple threads with `thread_data_t` struct 🟥:
```c
#define NUM_THREADS 5

void* print_hello(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;
    printf("Hello from thread %d with info %d\n", data->thread_id, data->info);
    pthread_exit(NULL); // can make it return a pointer if needed
}

pthread_t threads[NUM_THREADS];
thread_data_t thread_data_array[NUM_THREADS];

for (int i = 0; i < NUM_THREADS; i++) {
    thread_data_array[i].thread_id = i;
    thread_data_array[i].info = some_info;

    int rc = pthread_create(&threads[i], NULL, print_hello, (void *)&thread_data_array[i]);
    if (rc) {
        printf("Error:unable to create thread, %d\n", rc);
        exit(-1);
    }
}
```

> [!TIP]
> `rc` is the return code of `pthread_create()`, if it's non-zero, it indicates an error occurred during thread creation (could be due to resource limitations or invalid arguments), so it's good practice to check it

### Waiting for threads to finish with pthread_join()

What is exactly "waiting for a thread to finish"?  
When a thread is created, it runs concurrently with the main program and other threads and sometimes, the main program needs to wait for a thread to complete its task before proceeding, especially if the main program depends on the results produced by that thread. This step ensures that the main program does not continue executing until the specified thread has finished its execution.

To wait for a thread to finish, use `pthread_join()`, which takes two arguments:
- thread id of the thread to wait for
- pointer to the return value of the thread (can be NULL if not needed)

*e.g.,* to wait for a single thread 🟩:
```c
pthread_join(thread, NULL);
```
*e.g.,* waiting for 2 threads 🟨:
```c
pthread_join(threads[0], NULL);
pthread_join(threads[1], NULL);
```
*e.g.,* waiting for multiple threads in a loop 🟥:
```c
//...  after creating threads

for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
}
```

> [!TIP]
> can also put checks on pthread_join() return value to catch errors

## Mutex

Mutex is short for "mutual exclusion". It's a synchronization primitive used to protect shared resources from concurrent access by multiple threads - when a thread locks a mutex, other threads that try to lock the same mutex will be blocked until the mutex is unlocked

> [!CAUTION]
> when 2 threads have access to a shared resource (like a global variable or static variable in a function) and modify it concurrently it might lead to a "race condition", where the final value of the shared resource depends on the timing of the thread execution, which is unpredictable and can lead to inconsistent or incorrect results

Mutex help us avoid race through a thread safe multithread concurrency, by explicitly locking and unlocking the mutex around critical sections of code that access shared resources

### Using mutex in C with pthreads

1. initialize a mutex variable of type `pthread_mutex_t`

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

2. lock the mutex before accessing the shared resource

```c
pthread_mutex_lock(&mutex);
// critical section accessing shared resource
```

3. unlock the mutex after accessing the shared resource

```c
// end of critical section
pthread_mutex_unlock(&mutex);
```

### Example of using mutex

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int shared_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* increment_counter(void* arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[2];

    // Create two threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, increment_counter, NULL);
    }

    // Wait for both threads to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value: %d\n", shared_counter);
    pthread_mutex_destroy(&mutex);
    return 0;
}
```

> [!IMPORTANT]
> for every mutex initialized, need to destroy it at the end of its use with `pthread_mutex_destroy(&mutex);` to free resources (exactly like we do with malloc/free)

## Deadlocks

deadlock situation happen where two or more threads are blocked forever, each waiting for the other to release a resource which can occur when multiple threads acquire multiple locks in different orders, leading to a circular wait condition.

e.g., Thread A holds Lock 1 and waits for Lock 2, while Thread B holds Lock 2 and waits for Lock 1...

To avoid deadlocks:

$$\forall i,j, \text{ if } i \ne j, i \lt j \lor j \lt i$$
in other terms, always acquire multiple locks in a consistent global order across all threads

*it's always recommended to minimize the number of locks held simultaneously to reduce the risk of deadlocks*

## Condition variables

Condition variables are synchronization primitives that enable threads to wait for certain conditions to be met before proceeding, typically used in conjunction with mutexes to allow threads to wait for specific events or states to occur


**why mutex around condition variables?**  
The mutex is used to protect the shared condition variable and ensure that the condition is checked and modified atomically. When a thread waits on a condition variable, it releases the associated mutex while waiting, allowing other threads to acquire the mutex and modify the condition. When the waiting thread is signaled, it reacquires the mutex before proceeding, ensuring that the condition is still valid.

Atomically 3 steps are done:
1. release the mutex
2. sleep until con becomes true and signaled
3. relock the mutex

### Using condition variables in C with pthreads

1. initialize a condition variable of type `pthread_cond_t`

```c
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
```
2. wait on the condition variable (inside a mutex lock)

```c
pthread_mutex_lock(&mutex);
while (!condition) {
    pthread_cond_wait(&cond_var, &mutex);
}
// proceed when condition is met
pthread_mutex_unlock(&mutex);
```
3. signal the condition variable to wake up waiting threads

```c
pthread_mutex_lock(&mutex);
// update condition
pthread_cond_signal(&cond_var); // or pthread_cond_broadcast(&cond_var) to wake all waiting threads
pthread_mutex_unlock(&mutex);
```

### Buffer example producer-consumer

Consumer thred:  
* sleeps on empty buffer
* removes an item from buffer
* signals "not full" condition to producer
* 
Producer thread:
* sleeps on full buffer
* adds an item to buffer
* signals "not empty" condition to consumer

## Multithread model

Manager/worker model:

one thread (manager):
* receives requests
* assigns tasks to worker threads

multiple threads (workers), each:
* decode request
* handles it
* sends response

## Time evaluation

To evaluate the time, will be done terminal after compilation:

```bash
gcc -o my_program my_program.c -lpthread
time ./my_program
```

This will give output like:
```
real    0m0.027s
user    0m0.002s
sys     0m0.022s
```
Where:
- `real`: total elapsed time from start to finish of the program including all processes and threads
- `user`: total CPU time spent in user mode, so the time the CPU spent executing the program's own code
- `sys`: total CPU time spent in kernel mode (system calls, I/O operations)


## References

- [POSIX threads youtube tutorial](https://youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&si=yo0bdpxgxCfOJm9h)
- Advanced Programming course chapter I - POSIX threads