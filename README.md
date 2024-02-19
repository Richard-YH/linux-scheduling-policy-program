# Project Report: Scheduling Policy Demonstration Program

## Introduction

This project aims to implement a scheduling policy demonstration program called sched_demo. The program allows users to run multiple threads with different scheduling policies and observe their behaviors. This report provides a detailed overview of the implementation process and discusses the results of various test scenarios.

## Implementation Details

### Program Structure

The program is structured into two main sections: the main thread section and the worker thread section.

#### Main Thread Section

- Parses program arguments using `getopt(3)`.
- Sets CPU affinity of all threads to the same CPU.
- Creates `<num_threads>` worker threads specified by the option `-n`.
- Sets attributes such as scheduling inheritance, scheduling policy, and scheduling priority for each worker thread.
- Starts all threads simultaneously using `pthread_barrier_wait(3p)` for synchronization.
- Waits for all threads to finish execution.

#### Worker Thread Section

- Each worker thread waits for other threads to become ready before executing its task.
- Runs a loop for three times.
- Displays a message indicating it's running and performs busy work for `<time_wait>` seconds.
- Exits the function after completing the task.

### Implementation Language

The program is implemented using the C programming language.

### Compilation Environment

The program is compiled and tested on Ubuntu 22.04 AMD64.

### Dependencies

The program relies on the pthread library for multi-threading support.

### Code Organization

The source code is organized into separate files for clarity and modularity.

## Test Results

- **Testcase 1**: `./sched_demo -n 1 -t 0.5 -s NORMAL -p -1`
  - Result: Success!
  - Description: The program successfully runs one thread with a normal scheduling policy.
  
- **Testcase 2**: `./sched_demo -n 2 -t 0.5 -s FIFO,FIFO -p 10,20`
  - Result: Success!
  - Description: The program successfully runs two threads with FIFO scheduling policy and corresponding priorities.
  
- **Testcase 3**: `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30`
  - Result: Success!
  - Description: The program successfully runs three threads with a combination of normal and FIFO scheduling policies. Thread 1 runs with a normal policy, while threads 2 and 3 run with FIFO policies and different priorities.

- **Additional Testcase**: `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30`
  - Result: Success!
  - Description: The program successfully runs four threads with a combination of normal and FIFO scheduling policies. Threads 1 and 3 run with normal policies, while threads 2 and 4 run with FIFO policies and different priorities.

## Discussion

### Results Analysis: `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30`

- The result shows successful execution of three threads with different scheduling policies.
- Thread 1 runs with a normal scheduling policy (-1 priority) and completes its task.
- Threads 2 and 3 run with FIFO scheduling policy and priorities 10 and 30 respectively.
- The FIFO threads preempt the normal thread due to their higher priorities.

### Results Analysis: `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30`

- The result shows successful execution of four threads with a combination of normal and FIFO scheduling policies.
- Threads 1 and 3 run with normal scheduling policy and complete their tasks.
- Threads 2 and 4 run with FIFO scheduling policy and priorities 10 and 30 respectively.
- FIFO threads preempt normal threads during execution, demonstrating the behavior of real-time scheduling policies.

## Implementation of n-second-busy-waiting

### Methodology

In the busy work, I used the **`timespec`** structure to store timestamps and utilized **`clock_gettime`** to obtain the time. Then, I calculated the elapsed time using the formula **`elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1.0e9;`** to store the total seconds passed. It is essential to be precise to nanosecond because the Linux kernel's documentation on [CFS (Completely Fair Scheduler)](https://www.kernel.org/doc/Documentation/scheduler/sched-design-CFS.txt) (Completely Fair Scheduler) mentions that it measures expected CPU time in nanosecond units.

### Code Snippet
```c
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
```

# Conclusion

The scheduling policy demonstration program (sched_demo) successfully implements different scheduling policies and provides insights into thread behavior under various scenarios. The program's modular design and effective implementation ensure accurate scheduling policy management and thread synchronization.

This project report covers the implementation details, test results, and analysis of the scheduling policy demonstration program. It provides a comprehensive overview of the program's functionality and performance under different test scenarios.