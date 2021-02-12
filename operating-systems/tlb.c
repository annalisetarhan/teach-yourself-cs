#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define MICROSECS_IN_SECONDS 1000000
#define NANOSECS_IN_MICROS 1000

void tlb(int num_pages, int num_trials) {
    /* Create an array spanning the given number of pages */
    int total_bytes = num_pages * getpagesize();
    int elements = total_bytes / sizeof(int);

    int array[elements];
    int jump = getpagesize() / sizeof(int);
    
    for (int i = 0; i < elements; i++) {
        array[i] = 0;
    }
    
    struct timeval start_time;
    struct timeval end_time;
    
    /* Jump from one page to the next num_trials times, timing how long it takes */
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < num_trials; i++) {
        for (int j = 0; j < elements; j += jump) {
            array[j] += 1;
        }
    }
    gettimeofday(&end_time, NULL);
    
    /* Calculate elapsed time in microseconds */
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    /* Convert to nanoseconds before dividing */
    long time_per_iteration = elapsed_time * NANOSECS_IN_MICROS / num_trials / num_pages;
    
    printf("Pages: %i     Trials: %i     Time: %li\n", num_pages, num_trials, time_per_iteration);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("tlb needs two arguments: num_pages and num_trials\n");
    } else {
        tlb(atoi(argv[1]), atoi(argv[2]));
    }
}
