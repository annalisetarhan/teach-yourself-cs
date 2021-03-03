//
//  LockBasedConcurrentDataStructures.c
//  OSTEP
//
//  Created by Annalise Tarhan on 3/2/21.
//

#include <stdlib.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define ITERATIONS 1000
#define MICROSECS_IN_SECONDS 1000000
#define NUMCPUS 4

/* Question 1 */

void generic_time_measurement_function() {
    struct timeval start_time;
    struct timeval end_time;
        
    /* Get start time */
    gettimeofday(&start_time, NULL);
    
    // DO STUFF
    
    /* Get end time */
    gettimeofday(&end_time, NULL);

    /* Calculate elapsed time in microseconds */
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    
    // PRINT STUFF
}

/*
 QUESTION 2
 
 Results in microseconds per million incrementations on a quad-core machine:
 One thread: 21000
 Two threads: 54000
 Three threads: 63000
 Four threads: 60000
 Five or more: 71000
 
 The running time for more than four threads is remarkably consistent.
 I'd guess this is because after threads are running on all four CPUs,
 it behaves similarly to a sequential program.
 */

/* Counter struct */
typedef struct __counter_t {
    int value;
    pthread_mutex_t lock;
} counter_t;

void init(counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

int get(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    int rc = c->value;
    pthread_mutex_unlock(&c->lock);
    return rc;
}

/* Thread function */
void *count(void *counter) {
    for (int i = 0; i < ITERATIONS; i++) {
        increment(counter);
    }
    return NULL;
}

/* Timer function */
void measure_counter_time(int threads, counter_t *counter) {
    struct timeval start_time;
    struct timeval end_time;
    
    pthread_t thread_ptrs[threads];
    
    /* Get start time */
    gettimeofday(&start_time, NULL);
    
    /* Create threads */
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ptrs[i], NULL, count, counter);
    }
    
    /* Wait for threads to finish */
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ptrs[i], NULL);
    }
    
    /* Get end time */
    gettimeofday(&end_time, NULL);

    /* Calculate elapsed time in microseconds */
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    /* Divide by number of threads, since each thread increments an additional million times */
    long time_per_million_incrementations = elapsed_time / threads;
    
    printf("Running with %i threads takes approximately %li microseconds per million incrementations.\n", threads, time_per_million_incrementations);
}

void concurrent_counter() {
    for (int i = 1; i < 8; i++) {
        counter_t *counter = (counter_t *) malloc(sizeof(int) + sizeof(pthread_mutex_t));
        init(counter);
        measure_counter_time(i, counter);
        free(counter);
    }
}

/*
 QUESTION 3
 
 My results look nothing like those shown in the chapter.
 Specifically, the choice of threshold didn't seem to make any difference
 at all, and after the number of threads increased past two, those results
 were indistinguishable as well.
 */

/* Sloppy counter struct */
typedef struct __counter_t_sloppy {
    int global;
    pthread_mutex_t g_lock;
    int local[NUMCPUS];
    pthread_mutex_t l_lock[NUMCPUS];
    int threshold;
} counter_sloppy;

void init_sloppy(counter_sloppy *c, int threshold) {
    c->threshold = threshold;
    c->global = 0;
    pthread_mutex_init(&c->g_lock, NULL);
    for (int i = 0; i < NUMCPUS; i++) {
        c->local[i] = 0;
        pthread_mutex_init(&c->l_lock[i], NULL);
    }
}

void update_sloppy(counter_sloppy *c, int threadID, int amt) {
    int cpu = threadID % NUMCPUS;
    pthread_mutex_lock(&c->l_lock[cpu]);
    c->local[cpu] += amt;
    if (c->local[cpu] >= c->threshold) {    // Update global
        pthread_mutex_lock(&c->g_lock);
        c->global += c->local[cpu];
        pthread_mutex_unlock(&c->g_lock);
        c->local[cpu] = 0;
    }
    pthread_mutex_unlock(&c->l_lock[cpu]);
}

int get_sloppy(counter_sloppy *c) {
    pthread_mutex_lock(&c->g_lock);
    int val = c->global;
    pthread_mutex_unlock(&c->g_lock);
    return val;
}

/* Thread function */
void *count_sloppy(void *counter_sloppy) {
    for (int i = 0; i < ITERATIONS; i++) {
        increment(counter_sloppy);
    }
    return NULL;
}

/* Timer function */
void measure_sloppy_counter_time(int threads, counter_sloppy *counter, int threshold) {
    struct timeval start_time;
    struct timeval end_time;
    
    pthread_t thread_ptrs[threads];
    
    /* Get start time */
    gettimeofday(&start_time, NULL);
    
    /* Create threads */
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ptrs[i], NULL, count_sloppy, counter);
    }
    
    /* Wait for threads to finish */
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ptrs[i], NULL);
    }
    
    /* Get end time */
    gettimeofday(&end_time, NULL);

    /* Calculate elapsed time in microseconds */
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    /* Divide by number of threads, since each thread increments an additional million times */
    long time_per_million_incrementations = elapsed_time / threads;
    
    printf("Threads: %i   Threshold: %i   Time: %li\n", threads, threshold, time_per_million_incrementations);
}

void sloppy_counter() {
    for (int threads = 1; threads < 8; threads++) {
        for (int threshold = 2; threshold<1000; threshold *= 2) {
            counter_sloppy *counter = (counter_sloppy *) malloc((1+NUMCPUS*2)*sizeof(int) +    2*sizeof(pthread_mutex_t));
            init_sloppy(counter, threshold);
            measure_sloppy_counter_time(threads, counter, threshold);
            free(counter);
        }
        printf("\n");
    }
}

/*
 QUESTION 4
 
 It took extreme inputs for hand-over-hand to produce better results.
 In almost all cases, the original concurrent linked list performed better,
 but once the number of threads and the length of the list (iterations)
 were both very high, hand-over-hand was clearly better, sometimes
 dramatically so. For 1024 threads and a list with 1024 links, hand-
 over-hand was 18 times faster in one case and 77 times faster in another.
 The cutoff seemed to be 1024 threads and at least 256 iterations to
 make hand-over-hand consistently better. A sample of the results is below.
 */

typedef struct __node_t {
    int key;
    struct __node_t *next;
    pthread_mutex_t lock;
} hoh_node_t;

typedef struct __list_t {
    hoh_node_t *head;
    pthread_mutex_t headLock;
} hoh_list_t;

void Hoh_List_Init(hoh_list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->headLock, NULL);
}

void Hoh_List_Insert(hoh_list_t *L, int key) {
    // Allocate space for new node
    hoh_node_t *new = malloc(sizeof(hoh_node_t));
    if (new == NULL) {
        perror("malloc");
        return;
    }

    // Set new node's key and initialize node's lock
    new->key = key;
    pthread_mutex_init(&new->lock, NULL);

    // Update list's head while head is locked
    pthread_mutex_lock(&L->headLock);
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->headLock);
}

int Hoh_List_Lookup(hoh_list_t *L, int key) {
    pthread_mutex_t curLock, nextLock;
    
    // Lock the list while accessing head node
    pthread_mutex_lock(&L->headLock);
    hoh_node_t *cur = L->head;
    if (cur != NULL) {
        curLock = cur->lock;
        pthread_mutex_lock(&curLock);
        pthread_mutex_unlock(&L->headLock);
    } else {
        pthread_mutex_unlock(&L->headLock);
        return -1;
    }

    // Traverse list, locking hand-over-hand
    while (cur->next != NULL) {
        if (cur->key == key) {
            pthread_mutex_unlock(&curLock);
            return 0;   // Success
        }
        cur = cur->next;
        nextLock = cur->lock;
        pthread_mutex_lock(&nextLock);
        pthread_mutex_unlock(&curLock);
        curLock = nextLock;
    }
    // Check last node in list
    if (cur->key == key) {
        pthread_mutex_unlock(&curLock);
        return 0;   // Success
    }
    return -1;  // Failure
}

void Hoh_free(hoh_list_t *list) {
    hoh_node_t *cur = list->head;
    hoh_node_t *next = cur->next;
    while (next) {
        free(cur);
        cur = next;
        next = cur->next;
    }
    free(cur);
}

void handOverTest() {
    printf("Should be -1 -1 0\n");
    hoh_list_t *list = malloc(sizeof(hoh_list_t));
    Hoh_List_Init(list);
    printf("%d\n", Hoh_List_Lookup(list, 6));
    Hoh_List_Insert(list, 5);
    Hoh_List_Insert(list, 10);
    printf("%d\n", Hoh_List_Lookup(list, 6));
    printf("%d\n", Hoh_List_Lookup(list, 10));
}

typedef struct __conc_node_t{
    int key;
    struct __conc_node_t *next;
} conc_node_t;

typedef struct __conc_list_t {
    conc_node_t *head;
    pthread_mutex_t lock;
} conc_list_t;

void Conc_List_Init(conc_list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

int Conc_List_Insert(conc_list_t *L, int key) {
    pthread_mutex_lock(&L->lock);
    conc_node_t *new = malloc(sizeof(conc_node_t));
    if (new == NULL) {
        perror("malloc");
        pthread_mutex_unlock(&L->lock);
        return -1;  // Failure
    }
    new->key = key;
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock);
    return  0;
}

int Conc_List_Lookup(conc_list_t *L, int key) {
    pthread_mutex_lock(&L->lock);
    conc_node_t *cur = L->head;
    while (cur) {
        if (cur->key == key) {
            pthread_mutex_unlock(&L->lock);
            return 0;   // Success
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&L->lock);
    return -1; // Failure
}

void Conc_free(conc_list_t *list) {
    conc_node_t *cur = list->head;
    conc_node_t *next = cur->next;
    while (next) {
        free(cur);
        cur = next;
        next = cur->next;
    }
    free(cur);
}

// Because passing arguments to a thread function is hard.
// Instead, set this variable in the multiThreadTest function.
int test_iterations;

void *test_conc_list_time(void *list) {
    for (int i = 0; i < test_iterations; i++) {
        Conc_List_Insert(list, i);
    }
    
    for (int i = 0; i < test_iterations; i++) {
        Conc_List_Lookup(list, 0);
    }

    return NULL;
}

void *test_hoh_list_time(void *list) {
        
    for (int i = 0; i < test_iterations; i++) {
        Hoh_List_Insert(list, i);
    }
    
    for (int i = 0; i < test_iterations; i++) {
        Hoh_List_Lookup(list, 0);
    }
        
    return NULL;
}

void multiThreadTest(int threads, int iterations) {
    struct timeval start_time;
    struct timeval end_time;
    
    test_iterations = iterations;
    
    pthread_t thread_ptrs[threads];
    
    /* CONCURRENT LIST */
    
    /* Get start time */
    gettimeofday(&start_time, NULL);
    
    conc_list_t *conc_list = malloc(sizeof(conc_list_t));
    Conc_List_Init(conc_list);
    
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ptrs[i], NULL, test_conc_list_time, conc_list);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ptrs[i], NULL);
    }
    
    /* Get end time */
    gettimeofday(&end_time, NULL);
    Conc_free(conc_list);

    /* Calculate elapsed time in microseconds */
    long conc_elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
        
    
    /* HAND OVER HAND LIST */
    
    /* Get start time */
    gettimeofday(&start_time, NULL);
    
    hoh_list_t *hoh_list = malloc(sizeof(hoh_list_t));
    Hoh_List_Init(hoh_list);
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ptrs[i], NULL, test_hoh_list_time, hoh_list);
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ptrs[i], NULL);
    }
    
    /* Get end time */
    gettimeofday(&end_time, NULL);

    Hoh_free(hoh_list);

    /* Calculate elapsed time in microseconds */
    long hoh_elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
        
    float hoh_f = (float) hoh_elapsed_time;
    float conc_f = (float) conc_elapsed_time;
    
    if (hoh_elapsed_time > conc_elapsed_time) {
        printf("Threads: %i     Iterations: %i     Winner: CONC     Ratio: %f\n", threads, iterations, hoh_f/conc_f);
    } else {
        printf("Threads: %i     Iterations: %i     Winner: HOH     Ratio: %f\n", threads, iterations, conc_f/hoh_f);
    }

}

void multiThreadTestDriver() {
    for (int i = 8; i <= 1024; i *= 2) {
        multiThreadTest(1024, i);
    }
    for (int i = 4; i <= 1024; i *= 2) {
        for (int j = 8; j <= 1024; j *= 2) {
            multiThreadTest(i, j);
        }
        printf("\n");
    }
}

/*
 *** Cherry-picked ***
 Threads: 1024     Iterations: 256     Winner: HOH     Ratio: 1.670393
 Threads: 1024     Iterations: 512     Winner: HOH     Ratio: 1.418885
 Threads: 1024     Iterations: 1024     Winner: HOH     Ratio: 76.839310
 Threads: 1024     Iterations: 512     Winner: HOH     Ratio: 9.818523
 Threads: 1024     Iterations: 1024     Winner: HOH     Ratio: 17.835697
 Threads: 1024     Iterations: 256     Winner: HOH     Ratio: 12.453730
 Threads: 4096     Iterations: 256     Winner: HOH     Ratio: 11.588802
 Threads: 8192     Iterations: 256     Winner: HOH     Ratio: 6.491114
 Threads: 16384     Iterations: 256     Winner: HOH     Ratio: 2.512874
 Threads: 32768     Iterations: 256     Winner: HOH     Ratio: 1.211577
 Threads: 65536     Iterations: 256     Winner: HOH     Ratio: 1.370324

 *** Representative ***
 Threads: 4     Iterations: 8     Winner: HOH     Ratio: 1.958549
 Threads: 4     Iterations: 16     Winner: CONC     Ratio: 1.014218
 Threads: 4     Iterations: 32     Winner: CONC     Ratio: 2.319277
 Threads: 4     Iterations: 64     Winner: CONC     Ratio: 1.491848
 Threads: 4     Iterations: 128     Winner: CONC     Ratio: 1.407132
 Threads: 4     Iterations: 256     Winner: CONC     Ratio: 1.434115
 Threads: 4     Iterations: 512     Winner: CONC     Ratio: 1.818872
 Threads: 4     Iterations: 1024     Winner: CONC     Ratio: 2.752130

 Threads: 8     Iterations: 8     Winner: HOH     Ratio: 1.605000
 Threads: 8     Iterations: 16     Winner: CONC     Ratio: 1.376471
 Threads: 8     Iterations: 32     Winner: CONC     Ratio: 1.396970
 Threads: 8     Iterations: 64     Winner: HOH     Ratio: 1.261842
 Threads: 8     Iterations: 128     Winner: HOH     Ratio: 1.336797
 Threads: 8     Iterations: 256     Winner: CONC     Ratio: 1.429233
 Threads: 8     Iterations: 512     Winner: CONC     Ratio: 1.768593
 Threads: 8     Iterations: 1024     Winner: CONC     Ratio: 2.366040

 Threads: 16     Iterations: 8     Winner: HOH     Ratio: 1.417910
 Threads: 16     Iterations: 16     Winner: CONC     Ratio: 1.308540
 Threads: 16     Iterations: 32     Winner: CONC     Ratio: 1.035599
 Threads: 16     Iterations: 64     Winner: HOH     Ratio: 1.145304
 Threads: 16     Iterations: 128     Winner: CONC     Ratio: 1.194386
 Threads: 16     Iterations: 256     Winner: CONC     Ratio: 1.895049
 Threads: 16     Iterations: 512     Winner: CONC     Ratio: 1.928514
 Threads: 16     Iterations: 1024     Winner: CONC     Ratio: 1.821020

 Threads: 32     Iterations: 8     Winner: HOH     Ratio: 1.176718
 Threads: 32     Iterations: 16     Winner: CONC     Ratio: 1.260606
 Threads: 32     Iterations: 32     Winner: CONC     Ratio: 1.392739
 Threads: 32     Iterations: 64     Winner: HOH     Ratio: 1.168718
 Threads: 32     Iterations: 128     Winner: CONC     Ratio: 1.051445
 Threads: 32     Iterations: 256     Winner: CONC     Ratio: 1.930054
 Threads: 32     Iterations: 512     Winner: CONC     Ratio: 1.869176
 Threads: 32     Iterations: 1024     Winner: CONC     Ratio: 1.834972

 Threads: 64     Iterations: 8     Winner: CONC     Ratio: 1.050220
 Threads: 64     Iterations: 16     Winner: CONC     Ratio: 1.050617
 Threads: 64     Iterations: 32     Winner: CONC     Ratio: 1.449370
 Threads: 64     Iterations: 64     Winner: HOH     Ratio: 1.015510
 Threads: 64     Iterations: 128     Winner: CONC     Ratio: 1.262605
 Threads: 64     Iterations: 256     Winner: CONC     Ratio: 1.175963
 Threads: 64     Iterations: 512     Winner: CONC     Ratio: 11.318593
 Threads: 64     Iterations: 1024     Winner: CONC     Ratio: 1.938899

 Threads: 128     Iterations: 8     Winner: CONC     Ratio: 1.007728
 Threads: 128     Iterations: 16     Winner: HOH     Ratio: 1.061873
 Threads: 128     Iterations: 32     Winner: CONC     Ratio: 1.942167
 Threads: 128     Iterations: 64     Winner: CONC     Ratio: 1.594714
 Threads: 128     Iterations: 128     Winner: CONC     Ratio: 1.377767
 Threads: 128     Iterations: 256     Winner: CONC     Ratio: 1.547174
 Threads: 128     Iterations: 512     Winner: CONC     Ratio: 13.027803
 Threads: 128     Iterations: 1024     Winner: CONC     Ratio: 2.101078

 Threads: 256     Iterations: 8     Winner: HOH     Ratio: 1.172907
 Threads: 256     Iterations: 16     Winner: HOH     Ratio: 1.026544
 Threads: 256     Iterations: 32     Winner: CONC     Ratio: 1.215602
 Threads: 256     Iterations: 64     Winner: CONC     Ratio: 1.560992
 Threads: 256     Iterations: 128     Winner: CONC     Ratio: 1.516261
 Threads: 256     Iterations: 256     Winner: CONC     Ratio: 1.819652
 Threads: 256     Iterations: 512     Winner: CONC     Ratio: 15.627192
 Threads: 256     Iterations: 1024     Winner: CONC     Ratio: 7.819963

 Threads: 512     Iterations: 8     Winner: CONC     Ratio: 1.090046
 Threads: 512     Iterations: 16     Winner: CONC     Ratio: 1.054962
 Threads: 512     Iterations: 32     Winner: CONC     Ratio: 1.437788
 Threads: 512     Iterations: 64     Winner: CONC     Ratio: 1.066628
 Threads: 512     Iterations: 128     Winner: CONC     Ratio: 1.204806
 Threads: 512     Iterations: 256     Winner: CONC     Ratio: 1.667712
 Threads: 512     Iterations: 512     Winner: CONC     Ratio: 4.844328
 Threads: 512     Iterations: 1024     Winner: CONC     Ratio: 1.225094

 Threads: 1024     Iterations: 8     Winner: HOH     Ratio: 1.339430
 Threads: 1024     Iterations: 16     Winner: HOH     Ratio: 1.062295
 Threads: 1024     Iterations: 32     Winner: CONC     Ratio: 1.489675
 Threads: 1024     Iterations: 64     Winner: CONC     Ratio: 4.733219
 Threads: 1024     Iterations: 128     Winner: CONC     Ratio: 1.376643
 Threads: 1024     Iterations: 256     Winner: HOH     Ratio: 1.670393
 Threads: 1024     Iterations: 512     Winner: HOH     Ratio: 1.418885
 Threads: 1024     Iterations: 1024     Winner: HOH     Ratio: 76.839310
 
 Threads: 1024     Iterations: 8     Winner: HOH     Ratio: 1.048690
 Threads: 1024     Iterations: 16     Winner: CONC     Ratio: 1.009212
 Threads: 1024     Iterations: 32     Winner: CONC     Ratio: 1.252554
 Threads: 1024     Iterations: 64     Winner: HOH     Ratio: 1.636549
 Threads: 1024     Iterations: 128     Winner: CONC     Ratio: 5.194393
 Threads: 1024     Iterations: 256     Winner: CONC     Ratio: 1.924388
 Threads: 1024     Iterations: 512     Winner: HOH     Ratio: 9.818523
 Threads: 1024     Iterations: 1024     Winner: HOH     Ratio: 17.835697
 
 Threads: 1024     Iterations: 8     Winner: HOH     Ratio: 1.447797
 Threads: 1024     Iterations: 16     Winner: CONC     Ratio: 1.165207
 Threads: 1024     Iterations: 32     Winner: CONC     Ratio: 1.418734
 Threads: 1024     Iterations: 64     Winner: CONC     Ratio: 1.552672
 Threads: 1024     Iterations: 128     Winner: CONC     Ratio: 1.628665
 Threads: 1024     Iterations: 256     Winner: HOH     Ratio: 12.453730

 Threads: 2048     Iterations: 8     Winner: HOH     Ratio: 1.098741
 Threads: 2048     Iterations: 16     Winner: CONC     Ratio: 1.108420
 Threads: 2048     Iterations: 32     Winner: CONC     Ratio: 1.305544
 Threads: 2048     Iterations: 64     Winner: CONC     Ratio: 1.518773
 Threads: 2048     Iterations: 128     Winner: CONC     Ratio: 3.256235
 Threads: 2048     Iterations: 256     Winner: CONC     Ratio: 1.979028

 Threads: 4096     Iterations: 8     Winner: HOH     Ratio: 1.000946
 Threads: 4096     Iterations: 16     Winner: HOH     Ratio: 1.050526
 Threads: 4096     Iterations: 32     Winner: CONC     Ratio: 1.147547
 Threads: 4096     Iterations: 64     Winner: CONC     Ratio: 1.776267
 Threads: 4096     Iterations: 128     Winner: CONC     Ratio: 2.640177
 Threads: 4096     Iterations: 256     Winner: HOH     Ratio: 11.588802

 Threads: 8192     Iterations: 8     Winner: HOH     Ratio: 1.082797
 Threads: 8192     Iterations: 16     Winner: HOH     Ratio: 1.087329
 Threads: 8192     Iterations: 32     Winner: HOH     Ratio: 1.008505
 Threads: 8192     Iterations: 64     Winner: CONC     Ratio: 1.367201
 Threads: 8192     Iterations: 128     Winner: CONC     Ratio: 1.313273
 Threads: 8192     Iterations: 256     Winner: HOH     Ratio: 6.491114

 Threads: 16384     Iterations: 8     Winner: CONC     Ratio: 1.006192
 Threads: 16384     Iterations: 16     Winner: HOH     Ratio: 1.041316
 Threads: 16384     Iterations: 32     Winner: CONC     Ratio: 1.022941
 Threads: 16384     Iterations: 64     Winner: CONC     Ratio: 1.166290
 Threads: 16384     Iterations: 128     Winner: CONC     Ratio: 1.095457
 Threads: 16384     Iterations: 256     Winner: HOH     Ratio: 2.512874

 Threads: 32768     Iterations: 8     Winner: CONC     Ratio: 1.001539
 Threads: 32768     Iterations: 16     Winner: CONC     Ratio: 1.016485
 Threads: 32768     Iterations: 32     Winner: HOH     Ratio: 1.000225
 Threads: 32768     Iterations: 64     Winner: CONC     Ratio: 1.052890
 Threads: 32768     Iterations: 128     Winner: CONC     Ratio: 1.103193
 Threads: 32768     Iterations: 256     Winner: HOH     Ratio: 1.211577

 Threads: 65536     Iterations: 8     Winner: HOH     Ratio: 1.045945
 Threads: 65536     Iterations: 16     Winner: CONC     Ratio: 1.022824
 Threads: 65536     Iterations: 32     Winner: CONC     Ratio: 1.033409
 Threads: 65536     Iterations: 64     Winner: CONC     Ratio: 1.066306
 Threads: 65536     Iterations: 128     Winner: CONC     Ratio: 1.366853
 Threads: 65536     Iterations: 256     Winner: HOH     Ratio: 1.370324
 */
