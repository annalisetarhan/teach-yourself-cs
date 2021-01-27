//
//  LimitedDirectExecution.c
//  OSTEP
//
//  Created by Annalise Tarhan on 1/27/21.
//

#include <stdio.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define ITERATIONS 1000000
#define MICROSECS_IN_SECONDS 1000000
#define NANOSECS_IN_MICROS 1000

/*
 This measurement is accurate to the microsecond and works by
 marking the time before and after a large number of system calls.
 It doesn't account for anything else the CPU does during that time.
 
 The average time for system calls is around 450 nanoseconds.
 */
void measure_system_call() {
    struct timeval start_time;
    struct timeval end_time;
    int fd = open("dummy.txt", O_RDONLY|O_CREAT, S_IRWXU);
    char buf;
    
    /* Get start time, call system calls, get end time */
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        read(fd, &buf, 0);
    }
    gettimeofday(&end_time, NULL);

    /* Calculate elapsed time in microseconds */
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    /* Convert to nanoseconds before dividing */
    long time_per_iteration = elapsed_time * NANOSECS_IN_MICROS / ITERATIONS;
    
    printf("Each system call takes approximately %li nanoseconds.\n", time_per_iteration);
}

/*
 Unfortunately, this doesn't seem to be possible on macOS.
    "OS X does not export interfaces that identify processors or control
    thread placement—explicit thread to processor binding is not supported."
(And nothing has changed since OS X became macOS.)
 
 Just for practice, I've implemented it anyway, but the result is meaningless.
 */
void measure_context_switch() {
    int pipe1fd[2];
    int pipe2fd[2];
    char buf;
    if (pipe(pipe1fd) == -1) {
        fprintf(stderr, "Pipe failed\n");
    }
    if (pipe(pipe2fd) == -1) {
        fprintf(stderr, "Pipe failed\n");
    }
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        /* Child writes to pipe1, reads from pipe2 */
        close(pipe1fd[0]);
        close(pipe2fd[1]);
        for (int i = 0; i < ITERATIONS; i++) {
            write(pipe1fd[1], "Hi", 2);
            read(pipe2fd[0], &buf, 2);
        }
        close(pipe1fd[1]);
        close(pipe2fd[0]);
    } else {
        /* Parent tracks start and end times */
        struct timeval start_time;
        struct timeval end_time;
        /* Parent reads from pipe1, writes to pipe2 */
        close(pipe1fd[1]);
        close(pipe2fd[0]);
        gettimeofday(&start_time, NULL);
        for (int i = 0; i < ITERATIONS; i++) {
            read(pipe1fd[0], &buf, 2);
            write(pipe2fd[1], "Hi", 2);
        }
        gettimeofday(&end_time, NULL);
        close(pipe1fd[0]);
        close(pipe2fd[1]);
        
        /* Calculate elapsed time in microseconds */
        long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
        /* Convert to nanoseconds before dividing */
        long time_per_iteration = elapsed_time * NANOSECS_IN_MICROS / ITERATIONS;
        /* Each iteration included four system calls (read, write, read, write),
           which we calculated to take approximately 450 nanoseconds each. */
        long time_per_context_change = time_per_iteration - 4*450;
        printf("Each context change takes approximately %li nanoseconds.\n", time_per_context_change);
    }
}

int main(int argc, const char * argv[]) {
    measure_system_call();
    measure_context_switch();
    return 0;
}
