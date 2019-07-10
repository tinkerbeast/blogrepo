// stdlib headers
#include <stdio.h>
// syste headers
#include <pthread.h>
#include <signal.h>
// free standing headers
#include <stdatomic.h>
#include <stdlib.h>


#define THREAD_COUNT 2

static unsigned int loop_count = 0x8ffffff;

#ifdef BUILD_ATOMIC
static atomic_int shared_value = 100;
#else
static int shared_value = 100;
#endif

#ifdef BUILD_UNSAFE
void * threaded_unsafe_incrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        shared_value++;
    }

    return NULL;
}

void * threaded_unsafe_decrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        shared_value--;
    }

    return NULL;
}

void* (*incrementer)(void*) = threaded_unsafe_incrementer;
void* (*decrementer)(void*) = threaded_unsafe_decrementer;
#endif

#ifdef BUILD_ATOMIC
void * threaded_atomic_incrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        atomic_fetch_add_explicit(&shared_value, 1, memory_order_relaxed);
    }

    return NULL;
}

void * threaded_atomic_decrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        atomic_fetch_sub_explicit(&shared_value, 1, memory_order_relaxed);
    }

    return NULL;
}

void* (*incrementer)(void*) = threaded_atomic_incrementer;
void* (*decrementer)(void*) = threaded_atomic_decrementer;
#endif

#ifdef BUILD_MUTEX
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * threaded_mutex_incrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        if (pthread_mutex_lock(&mutex) != 0) goto ERROR;

        shared_value++;
        
        if (pthread_mutex_unlock(&mutex) != 0) goto ERROR;
    }

    return NULL;

ERROR:
    perror("mutex lock / unlock error");
    exit(1);
}

void * threaded_mutex_decrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        if (pthread_mutex_lock(&mutex) != 0) goto ERROR;

        shared_value--;
        
        if (pthread_mutex_unlock(&mutex) != 0) goto ERROR;
    }

    return NULL;

ERROR:
    perror("mutex lock / unlock error");
    exit(1);
}

void* (*incrementer)(void*) = threaded_mutex_incrementer;
void* (*decrementer)(void*) = threaded_mutex_decrementer;
#endif


#ifdef BUILD_FLAGGED
sig_atomic_t shared_flag = 0;

void * threaded_flagged_incrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
        while (1) {
            if (shared_flag == 0) {
                shared_value++;
                shared_flag = 1;
                break;
            }
        }
    }

    return NULL;
}

void * threaded_flagged_decrementer(void * param)
{
    for (int i = 0; i < loop_count; i++) {
         while (1) {
            if (shared_flag != 0) {
                shared_value--;
                shared_flag = 0;
                break;
            }
        }       
    }

    return NULL;
}


void* (*incrementer)(void*) = threaded_flagged_incrementer;
void* (*decrementer)(void*) = threaded_flagged_decrementer;
#endif


int main(int argc, char * argv[])
{
    // Get the loop_count parameter
    if (argc < 2) {
        printf("Usage: %s LOOP_COUNT\n", argv[0]);
        return EXIT_FAILURE;
    }
    loop_count = atoi(argv[1]);

    int ret = -1;
    pthread_t thread[THREAD_COUNT];

    printf("Value before starting: %d\n", shared_value);

    // Start all the threads together
    for (int i = 0; i < THREAD_COUNT; i++) {

        // Make half the threads incrementer and the other half decrementer
        if (i % 2 == 0) {
            ret = pthread_create(&thread[i], NULL, incrementer, NULL);
        } else {
            ret = pthread_create(&thread[i], NULL, decrementer, NULL);
        }
        if (ret != 0) {
            perror("Failed to create threads");
            return EXIT_FAILURE;
        }
    }

    // Wait for all the threads to complete
    for (int i = 0; i < THREAD_COUNT; i++) {
        void * return_param = NULL;
        ret = pthread_join(thread[i], &return_param);
        if (ret != 0) {
            perror("Failed to join threads");
            return EXIT_FAILURE;
        }
    }

    printf("Value after ending: %d\n", shared_value);

    return EXIT_SUCCESS;
}
