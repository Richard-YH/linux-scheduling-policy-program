#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

pthread_barrier_t barrier;
double time_wait = -1.0;

typedef struct
{
    pthread_t thread_id; // ID returned by pthread_create()
    int thread_num;      // Application-defined thread #
    int sched_policy;    // Scheduling policy (e.g., SCHED_FIFO)
    int sched_priority;  // Priority within the scheduling policy
} thread_info_t;

int usage_explain(const char *program_name)
{
    int result = fprintf(stderr,
                         "Usage:\n"
                         "\t%s -n <num_thread> -t <time_wait> -s <policies> -p <priorities>\n"
                         "Options:\n"
                         "\t-n <num_threads>  Number of threads to run simultaneously\n"
                         "\t-t <time_wait>    Duration of \"busy\" period\n"
                         "\t-s <policies>     Scheduling policy for each thread,\n"
                         "\t                    currently only NORMAL(SCHED_NORMAL) and FIFO(SCHED_FIFO)\n"
                         "\t                    scheduling policies are supported.\n"
                         "\t-p <priorities>   Real-time thread priority for real-time threads\n"
                         "Example:\n"
                         "\t%s -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30\n",
                         program_name, program_name);

    return result;
}

void *thread_func(void *args)
{
    thread_info_t *thread_info = args;

    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(&barrier);

    /* 2. Do the task */
    for (int i = 0; i < 3; i++)
    {
        printf("Thread %d is running\n", thread_info->thread_num);
        /* Busy for <time_wait> seconds */
        busy_work(time_wait);
    }

    /* 3. Exit the function  */
    return 0;
}

void busy_work(double seconds)
{
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    do
    {
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                       (end_time.tv_nsec - start_time.tv_nsec) / 1.0e9;
    } while (elapsed_time < seconds);
}

int main(int argc, char **argv)
{
    int option;
    int num_threads = -1;
    char *policies = NULL;
    char *priorities = NULL;

    // 1. Parse program arguments
    while ((option = getopt(argc, argv, "n:t:s:p:h")) != -1)
    {
        switch (option)
        {
        case 'h':
            usage_explain(argv[0]);
            printf("help\n");
            exit(0);
        case 'n':
            num_threads = strtol(optarg, NULL, 10);
            if (num_threads == 0)
            {
                fprintf(stderr, "strtol: no digits were found\n");
                exit(1);
            }
            break;
        case 'p':
            priorities = optarg;
            break;
        case 's':
            policies = optarg;
            break;
        case 't':
            if (sscanf(optarg, "%lf", &time_wait) != 1)
            {
                fprintf(stderr, "sscanf: error parsing time_wait %s\n", optarg);
                exit(1);
            }
            break;
        default:
            break;
        }
    }

    if (num_threads == -1 || !policies || !priorities || time_wait == -1.0)
    {
        usage_explain(argv[0]);
        exit(1);
    }

    // 2. Create <num_threads> worker threads
    thread_info_t thread_info[num_threads];

    // Policy parsing
    char *policy_tokens[num_threads];
    char *policy_token = strtok(policies, ",");
    int policy_count = 0;

    while (policy_token != NULL && policy_count < num_threads)
    {
        policy_tokens[policy_count] = policy_token;
        // printf("policy_token: %s\n", policy_token);

        if (!strcmp(policy_token, "FIFO"))
        {
            thread_info[policy_count].sched_policy = SCHED_FIFO;
        }
        else if (strcmp(policy_token, "NORMAL") && strcmp(policy_token, "OTHER"))
        {
            fprintf(stderr, "Policy \"%s\" is not one of the supported policies\n", policy_token);
            exit(1);
        }
        else
        {
            thread_info[policy_count].sched_policy = SCHED_OTHER;
        }

        policy_token = strtok(NULL, ",");
        policy_count++;
    }

    // Priority parsing
    char *priority_tokens[num_threads];
    char *priority_token = strtok(priorities, ",");
    int priority_count = 0;

    while (priority_token != NULL && priority_count < num_threads)
    {
        priority_tokens[priority_count] = priority_token;
        priority_token = strtok(NULL, ",");
        priority_count++;
    }

    // Check if the number of policies and priorities match the number of threads
    if (policy_count != num_threads || priority_count != num_threads)
    {
        fprintf(stderr, "Error: Number of policies or priorities does not match the number of threads\n");
        exit(1);
    }

    // Assign thread_info_t struct
    for (int i = 0; i < num_threads; i++)
    {
        thread_info[i].thread_num = i;
        thread_info[i].sched_priority = atoi(priority_tokens[i]);
    }

    // 3. Set CPU affinity
    cpu_set_t cpus;
    pthread_attr_t attr[num_threads];
    struct sched_param param[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_attr_init(&attr[i]);

        int cpu = 0;
        CPU_ZERO(&cpus);
        CPU_SET(cpu, &cpus);
        pthread_attr_setaffinity_np(&attr[i], sizeof(cpu_set_t), &cpus);
    }

    // 4. Set scheduling policy and priority
    // attributes such as scheduling inheritance, scheduling policy and scheduling priority must be set
    int priority_max;
    int priority_min;

    pthread_barrier_init(&barrier, NULL, num_threads);
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED))
        {
            perror("pthread_attr_setinheritsched");
        }
        if (pthread_attr_setschedpolicy(&attr[i], thread_info[i].sched_policy))
        {
            perror("pthread_attr_setschedpolicy");
        }

        if (thread_info[i].sched_policy == SCHED_OTHER)
        {
            thread_info[i].sched_priority = 0; // SCHED_NORMAL priority range: [0, 0] not -1
        }

        priority_max = sched_get_priority_max(thread_info[i].sched_policy);
        priority_min = sched_get_priority_min(thread_info[i].sched_policy);
        if (thread_info[i].sched_priority > priority_max || thread_info[i].sched_priority < priority_min)
        {
            fprintf(stderr, "Error: Priority %d is not in the range of [%d, %d]\n",
                    thread_info[i].sched_priority, priority_min, priority_max);
            exit(1);
        }

        param[i].sched_priority = thread_info[i].sched_priority;
        if (pthread_attr_setschedparam(&attr[i], &param[i]))
        {
            perror("pthread_attr_setschedparam");
        }
    }

    //  5. Start all threads at once
    pthread_barrier_init(&barrier, NULL, num_threads);
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&thread_info[i].thread_id, &attr[i], thread_func, &thread_info[i]);
    }

    // 6. Wait for all threads to complete
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(thread_info[i].thread_id, NULL);
    }
    pthread_barrier_destroy(&barrier);

    return 0;
}