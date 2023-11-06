#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

typedef struct {
    pthread_t thread_id;       // ID returned by pthread_create()
    int thread_num;            // Application-defined thread #
    int sched_policy;          // Scheduling policy (e.g., SCHED_FIFO)
    int sched_priority;        // Priority within the scheduling policy
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

int main(int argc, char **argv)
{
    int option;
    int num_threads = -1;
    char *policies = NULL;
    char *priorities = NULL;
    double time_wait = -1.0;

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
                printf("num_threads: %d\n", num_threads);
                if (num_threads == 0)
                {
                    fprintf(stderr, "strtol: no digits were found\n");
                    exit(1);
                }
                break;
            case 'p':
                priorities = optarg;
                printf("priorities: %s\n", priorities);
                break;
            case 's':
                policies = optarg;
                printf("policies: %s\n", policies);
                break;
            case 't':
                if (sscanf(optarg, "%lf", &time_wait) != 1)
                {
                    fprintf(stderr, "sscanf: error parsing time_wait %s\n", optarg);
                    exit(1);
                }
                printf("time_wait: %lf\n", time_wait);
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

    while (policy_token != NULL && policy_count < num_threads) {
        policy_tokens[policy_count] = policy_token;
        printf("policy_token: %s\n", policy_token);

        if (!strcmp(policy_token, "FIFO")) {
            thread_info[policy_count].sched_policy = SCHED_FIFO; 
        } else if (strcmp(policy_token, "NORMAL") && strcmp(policy_token, "OTHER")) {
            fprintf(stderr, "Policy \"%s\" is not one of the supported policies\n", policy_token);
            exit(1);
        } else {
            thread_info[policy_count].sched_policy = SCHED_OTHER; 
        }

        policy_token = strtok(NULL, ",");
        policy_count++;
    }

    // Priority parsing
    char *priority_tokens[num_threads]; 
    char *priority_token = strtok(priorities, ",");
    int priority_count = 0;

    while (priority_token != NULL && priority_count < num_threads) {
        priority_tokens[priority_count] = priority_token;
        priority_token = strtok(NULL, ",");
        priority_count++;
    }

    // Check if the number of policies and priorities match the number of threads
    if (policy_count != num_threads || priority_count != num_threads) {
        fprintf(stderr, "Error: Number of policies or priorities does not match the number of threads\n");
        exit(1);
    }

    // Assign thread_info_t struct
    for (int i = 0; i < num_threads; i++) {
        thread_info[i].thread_num = i;
        // thread_info[i].sched_policy = atoi(policy_tokens[i]); 
        
        thread_info[i].sched_priority = atoi(priority_tokens[i]); 
    }

    for (int i = 0; i < num_threads; i++) {
        printf("thread_info[%d].thread_num: %d\n", i, thread_info[i].thread_num);
        printf("thread_info[%d].sched_policy: %d\n", i, thread_info[i].sched_policy);
        printf("thread_info[%d].sched_priority: %d\n", i, thread_info[i].sched_priority);
    }
        
    // 3. Set CPU affinity 
    




    // for (int i = 0; i < <num_threads>; i++) {
        /* 4. Set the attributes to each thread
    } */
    

    // 5. Start all threads at once 

    // 6. Wait for all threads to finish  


    // thread_func(num_threads, policies, priorities, time_wait);
    return 0;
}