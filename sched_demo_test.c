#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

typedef struct {
    pthread_t thread_id;       // ID returned by pthread_create()
    int thread_num;            // Application-defined thread #
    int sched_policy;          // Scheduling policy (e.g., SCHED_FIFO)
    int sched_priority;        // Priority within the scheduling policy
    pthread_barrier_t *barrier; // Barrier for synchronizing thread start
} thread_info_t;

// Thread start function
void *thread_start(void *arg) {
    thread_info_t *tinfo = arg;

    // Wait at the barrier until all threads are ready to start
    pthread_barrier_wait(tinfo->barrier);

    // Thread work goes here...

    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = 0; // Number of worker threads
    int opt;

    // 1. Parse program arguments
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s -n <num_threads>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads\n");
        exit(EXIT_FAILURE);
    }

    // Initialize barrier
    pthread_barrier_t start_barrier;
    pthread_barrier_init(&start_barrier, NULL, num_threads);

    // Allocate memory for thread_info_t structures
    thread_info_t *tinfo = calloc(num_threads, sizeof(*tinfo));

    // 2. Create <num_threads> worker threads
    for (int i = 0; i < num_threads; i++) {
        tinfo[i].thread_num = i + 1; // Thread numbering starts at 1

        // 4. Set the attributes to each thread
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // Set scheduling policy and priority in attr
        struct sched_param sched_param;
        sched_param.sched_priority = ...; // Set the priority
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // Example policy
        pthread_attr_setschedparam(&attr, &sched_param);
        // Note: You might need to set other attributes or use different values

        tinfo[i].sched_policy = SCHED_FIFO; // Example policy
        tinfo[i].sched_priority = sched_param.sched_priority;
        tinfo[i].barrier = &start_barrier;

        // 3. Set CPU affinity (optional step)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset); // Assign to CPU 0, as an example
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

        // Now, create the thread with the attributes
        pthread_create(&tinfo[i].thread_id, &attr, thread_start, &tinfo[i]);

        // Destroy the thread attributes object, since it's no longer needed
        pthread_attr_destroy(&attr);
    }

    // 6. Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tinfo[i].thread_id, NULL);
    }

    // Clean up barrier
    pthread_barrier_destroy(&start_barrier);

    // Free thread info
    free(tinfo);

    return 0;
}

