//
//  Semaphores.c
//  OSTEP
//
//  Created by Annalise Tarhan on 3/5/21.
//

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include <string.h>


/* Wrapper functions that check for error codes (and simplify init) */
sem_t *Sem_open(char *name, int num) {
    sem_t *semaphore = sem_open(name, O_CREAT|O_EXCL, S_IRUSR, num);
    if (semaphore == SEM_FAILED) {
        printf("%s\n", strerror(errno));
    }
    assert(semaphore != SEM_FAILED);
    return semaphore;
}

void Sem_wait(sem_t *semaphore) {
   assert(sem_wait(semaphore) == 0);
}

void Sem_post(sem_t *semaphore) {
    assert(sem_post(semaphore) == 0);
}

void Sem_close(sem_t *semaphore, char *name) {
    assert(sem_close(semaphore) == 0);
    sem_unlink(name);
}

/* Because sometimes these things seem to hang around */
void clear_all() {
    printf("%i", sem_unlink("/sem"));
    printf("%i", sem_unlink("/sem1"));
    printf("%i", sem_unlink("/sem2"));
    printf("%i", sem_unlink("/flood"));
    printf("%i", sem_unlink("/lock"));
    printf("%i", sem_unlink("/access_lock"));
    printf("%i", sem_unlink("/update_lock"));
    printf("%i", sem_unlink("/inner_lock"));
    printf("%i", sem_unlink("/outer_lock"));
    printf("%i", sem_unlink("/reader_count_lock"));
}





/*
 Question 1 - Fork/Join
 
 Uses one semaphore, opened by the parent. Parent creates a child thread,
 then waits for the semaphore. The child posts the semaphore, allowing the
 parent to proceed. The parent doesn't use join to wait for the child and
 the sleep guarantees that the child doesn't finish first by coincidence.
 */
sem_t *semaphore;

void *child(void *arg) {
    printf("child\n");
    sleep(1);
    Sem_post(semaphore);
    return NULL;
}

void fork_join() {
    pthread_t p;
    semaphore = Sem_open("/sem", 0);
    printf("parent:begin\n");
    pthread_create(&p, NULL, child, NULL);
    Sem_wait(semaphore);
    printf("parent: end\n");
    Sem_close(semaphore, "/sem");
}





/*
 Question 2 - Rendezvous
 
 This solution uses two semaphores to guarantee that both threads
 begin before either finishes. Each child waits posts one and waits
 for the other, and since the initial values of the semaphores is zero,
 neither can finish until the other has started.
 */

sem_t *s1, *s2;

void *child_1(void *arg) {
    printf("child 1: before\n");
    sleep(1);
    Sem_post(s1);
    Sem_wait(s2);
    printf("child 1: after\n");
    return NULL;
}

void *child_2(void *arg) {
    printf("child 2: before\n");
    Sem_post(s2);
    Sem_wait(s1);
    printf("child 2: after\n");
    return NULL;
}

void rendezvous() {
    pthread_t p1, p2;
    printf("parent: begin\n");
    s1 = Sem_open("/sem1", 0);
    s2 = Sem_open("/sem2", 0);
    pthread_create(&p1, NULL, child_1, NULL);
    pthread_create(&p2, NULL, child_2, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("parent: end\n");
    Sem_close(s1, "/sem1");
    Sem_close(s2, "/sem2");
}





/*
 Question 3 - Barrier Synchronization
 
 The barrier struct uses two semaphores and two ints.
 The first semaphore is the one all the child threads wait for, but
 nothing is posted to it until all the child threads have arrived.
 When each does, it waits for the lock semaphore, then updates the
 int that tracks the number of threads that have arrived. Then, it
 checks if the number of arrived threads equals the total number of
 threads and if so, it opens the floodgates, posting to the floodgate
 semaphore a number of times equal to the number of threads. Finally,
 it joins all the other threads waiting for the floodgate, at which
 point all the threads return from barrier and exit.
 
 The lock semaphore ensures that arrived_threads will be updated
 properly, and the sleep(1) before the floodgate opening makes it clear
 that none of the threads can cut in line and return from barrier before
 the last thread to arrive opens the floodgates.
 */

typedef struct __barrier_t {
    sem_t *floodgate, *lock;
    int total_threads, arrived_threads;
    
} barrier_t;

barrier_t bar;

void barrier_init(barrier_t *bar, int num_threads) {
    bar->floodgate = Sem_open("/flood", 0);
    bar->lock = Sem_open("/lock", 1);
    bar->total_threads = num_threads;
    bar->arrived_threads = 0;
}

void barrier_shutdown(barrier_t *bar) {
    Sem_close(bar->floodgate, "/flood");
    Sem_close(bar->lock, "/lock");
}

void barrier(barrier_t *bar) {
    Sem_wait(bar->lock);
    bar->arrived_threads++;
    if (bar->arrived_threads == bar->total_threads) {
        sleep(1);
        for (int i = 0; i < bar->total_threads; i++) {    // Open floodgates
            Sem_post(bar->floodgate);
        }
    }
    Sem_post(bar->lock);
    Sem_wait(bar->floodgate);
}

typedef struct __tinfo_t {
    int thread_id;
} tinfo_t;

void *child_bar(void *arg) {
    tinfo_t *t = (tinfo_t *) arg;
    printf("child %d: before\n", t->thread_id);
    barrier(&bar);
    printf("child %d:after\n", t->thread_id);
    return NULL;
}

void barrier_synch(int num_threads) {
    pthread_t p[num_threads];
    tinfo_t t[num_threads];
    
    printf("parent: begin\n");
    barrier_init(&bar, num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        t[i].thread_id = i;
        pthread_create(&p[i], NULL, child_bar, &t[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(p[i], NULL);
    }
    
    barrier_shutdown(&bar);
    
    printf("parent: end\n");
}





/*
 Question 4 - Reader/Writer
 
 Basically what's in the book.
 */

char *access_lock_name = "/access_lock";
char *update_lock_name = "/update_lock";

typedef struct __rwlock_t {
    int num_readers;
    sem_t *access_lock, *num_readers_update_lock;
} rwlock_t;


void rwlock_init(rwlock_t *rw) {
    rw->access_lock = Sem_open(access_lock_name, 1);
    rw->num_readers_update_lock = Sem_open(update_lock_name, 1);
    rw->num_readers = 0;
}

void rwlock_shutdown(rwlock_t *rw) {
    Sem_close(rw->access_lock, access_lock_name);
    Sem_close(rw->num_readers_update_lock, update_lock_name);
    sem_unlink(access_lock_name);
    sem_unlink(update_lock_name);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    Sem_wait(rw->num_readers_update_lock);
    rw->num_readers++;
    if (rw->num_readers == 1) {
        Sem_wait(rw->access_lock);
    }
    Sem_post(rw->num_readers_update_lock);
}

void rwlock_release_readlock(rwlock_t *rw) {
    Sem_wait(rw->num_readers_update_lock);
    rw->num_readers--;
    if (rw->num_readers == 0) {
        sleep(1);
        Sem_post(rw->access_lock);
    }
    Sem_post(rw->num_readers_update_lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    Sem_wait(rw->access_lock);
}

void rwlock_release_writelock(rwlock_t *rw) {
    sleep(1);
    Sem_post(rw->access_lock);
}

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
    rwlock_acquire_readlock(&lock);
    printf("read %d\n", value);
    rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
    rwlock_acquire_writelock(&lock);
    value++;
    printf("write %d\n", value);
    rwlock_release_writelock(&lock);
    }
    return NULL;
}

void reader_writer(int num_readers, int num_writers, int num_loops) {
    loops = num_loops;
    pthread_t pr[num_readers], pw[num_writers];
    rwlock_init(&lock);

    printf("begin\n");
    
    for (int i = 0; i < num_readers; i++) {
        pthread_create(&pr[i], NULL, reader, NULL);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_create(&pw[i], NULL, writer, NULL);
    }
    for (int i = 0; i < num_readers; i++) {
        pthread_join(pr[i], NULL);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_join(pw[i], NULL);
    }
    
    rwlock_shutdown(&lock);

    printf("end: value %d\n", value);
}





/*
 Question 5
 
 This version uses three semaphores: an outer_lock, an inner_lock, and a reader_count_lock.
 
 Both readers and writers must acquire the outer_lock before beginning. The readers,
 after acquiring the outer_lock, check if they are the first reader (while holding the
 reader_count_lock) and acquire the inner_lock if so. Then, before reading, they release
 the outer_lock. After reading, they check if they are the last reader (again, holding the
 reader_count_lock) and release the inner_lock if they are. Writers are simpler. They
 acquire the outer_lock, then the inner_lock. They read, then reverse the two locks in
 reverse order, inner_lock first and outer_lock second.
 
 The asymmetry between readers and writers is that readers release the outer_lock as soon
 as they acquire the inner lock, allowing other readers and writers in behind them. Writers,
 by contrast, keep that lock until they are finished writing. This reflects the fact that
 readers don't need exclusive access but writers do.
 */

char *inner_lock_name = "/inner_lock";
char *outer_lock_name = "/outer_lock";
char *reader_count_lock_name = "/reader_count_lock";

typedef struct __ns_rwlock_t {
    int num_readers;
    sem_t *outer_lock, *inner_lock, *reader_count_lock;
} ns_rwlock_t;

void ns_rwlock_init(ns_rwlock_t *rw) {
    rw->num_readers = 0;
    rw->outer_lock = Sem_open(outer_lock_name, 1);
    rw->inner_lock = Sem_open(inner_lock_name, 1);
    rw->reader_count_lock = Sem_open(reader_count_lock_name, 1);
}

void ns_rwlock_shutdown(ns_rwlock_t *rw) {
    Sem_close(rw->outer_lock, outer_lock_name);
    Sem_close(rw->inner_lock, inner_lock_name);
    Sem_close(rw->reader_count_lock, reader_count_lock_name);
    sem_unlink(outer_lock_name);
    sem_unlink(inner_lock_name);
    sem_unlink(reader_count_lock_name);
}

void ns_rwlock_acquire_readlock(ns_rwlock_t *rw) {
    Sem_wait(rw->outer_lock);
    Sem_wait(rw->reader_count_lock);
    rw->num_readers++;
    if (rw->num_readers == 1) {
        Sem_wait(rw->inner_lock);
    }
    Sem_post(rw->reader_count_lock);
    Sem_post(rw->outer_lock);
}

void ns_rwlock_release_readlock(ns_rwlock_t *rw) {
    Sem_wait(rw->reader_count_lock);
    rw->num_readers--;
    if (rw->num_readers == 0) {
        Sem_post(rw->inner_lock);
    }
    Sem_post(rw->reader_count_lock);
}

void ns_rwlock_acquire_writelock(ns_rwlock_t *rw) {
    Sem_wait(rw->outer_lock);
    Sem_wait(rw->inner_lock);
}

void ns_rwlock_release_writelock(ns_rwlock_t *rw) {
    Sem_post(rw->inner_lock);
    Sem_post(rw->outer_lock);
}

int ns_loops;
int ns_value = 0;

ns_rwlock_t ns_lock;

void *ns_reader(void *arg) {
    int i;
    for (i = 0; i < ns_loops; i++) {
    ns_rwlock_acquire_readlock(&ns_lock);
    printf("read %d\n", ns_value);
    ns_rwlock_release_readlock(&ns_lock);
    }
    return NULL;
}

void *ns_writer(void *arg) {
    int i;
    for (i = 0; i < ns_loops; i++) {
    ns_rwlock_acquire_writelock(&ns_lock);
    ns_value++;
    printf("write %d\n", ns_value);
    ns_rwlock_release_writelock(&ns_lock);
    }
    return NULL;
}

void reader_writer_nostarve(int num_readers, int num_writers, int num_loops) {
    ns_loops = num_loops;
    pthread_t pr[num_readers], pw[num_writers];
    ns_rwlock_init(&ns_lock);

    printf("begin\n");
    
    for (int i = 0; i < num_readers; i++) {
        pthread_create(&pr[i], NULL, ns_reader, NULL);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_create(&pw[i], NULL, ns_writer, NULL);
    }
    for (int i = 0; i < num_readers; i++) {
        pthread_join(pr[i], NULL);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_join(pw[i], NULL);
    }
    
    ns_rwlock_shutdown(&ns_lock);

    printf("end: value %d\n", ns_value);
}

