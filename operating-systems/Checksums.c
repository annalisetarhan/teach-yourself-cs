//
//  Checksums.c
//  OSTEP
//
//  Created by Annalise Tarhan on 3/18/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define MICROSECS_IN_SECONDS 1000000

/*
 Question 1
 
 This optionally prints out the previous checksum, the bits in the
 new character, and the new checksum for easy visual inspection.
 */

void check_xor(char *file_name, int show_steps) {
    unsigned char checksum = 0;
    char buffer[1];
    int fd = open(file_name, O_RDONLY);
    
    if (fd < 0) {
        perror("couldn't open file");
        exit(EXIT_FAILURE);
    }
    
    /* XOR each byte in file with checksum char */
    while (read(fd, buffer, 1) > 0) {
        if (show_steps) {
            printf("Char: %c\n", buffer[0]);
        
            // Print previous checksum
            printf("Prev checksum: ");
            for (int i = 0; i < 8; i++) {
                printf("%d", !!((checksum<<i) & 0x8));
            }
            // Print char's byte representation
            printf("\nChar bits:     ");
            for (int i = 0; i < 8; i++) {
                printf("%d", !!((buffer[0]<<i) & 0x8));
            }
        }
        
        // Do actual work
        checksum ^= buffer[0];

        if (show_steps) {
            // Print new checksum
            printf("\nNew checksum:  ");
            for (int i = 0; i < 8; i++) {
                printf("%d", !!((checksum<<i) & 0x8));  // Changes any non-zero value to one
            }
            printf("\n\n");
        }
    }
    
    // Print checksum
    printf("Final checksum for %s: ", file_name);
    for (int i = 0; i < 8; i++) {
        printf("%d", !!((checksum<<i) & 0x8));
    }
    printf("\n\n");
    
    close(fd);
    return;
}

/*
 Question 2
 
 This version operates on ASCII values, not the integer interpretation
 of the raw bytes. I'm not at all sure that's standard.
 */

void check_fletcher(char *file_name, int show_steps) {
    unsigned int s1 = 0;
    unsigned int s2 = 0;
    char buffer[1];
    unsigned char c;
    
    // Open file
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("couldn't open file");
        exit(EXIT_FAILURE);
    }
    
    // Read file and compute checksums
    while (read(fd, buffer, 1) > 0) {
        c = buffer[0];
        
        // Print previous checksums
        if (show_steps) {
            printf("Char: %c\n", c);
            printf("Prev s1: %i\n", s1);
            printf("Prev s2: %i\n", s2);
        }
        
        // Do actual work
        s1 = (s1 + c) % 255;
        s2 = (s2 + s1) % 255;

        // Print new checksums
        if (show_steps) {
            printf("New s1: %i\n", s1);
            printf("New s2: %i\n", s2);
            printf("\n\n");
        }
    }
    
    // Print result
    printf("Final checksums for %s:   (%i,  %i)", file_name, s1, s2);
    printf("\n\n");
    
    close(fd);
    return;
}

/*
 Question 3
 
 My implementations of XOR and Fletcher checksum calculations yield
 similar results, with XOR being only very slightly faster. XOR's tiny
 speed advantage comes at the cost of not distinguishing between different
 byte orderings.
 */

void test_checksums(char *file_name) {
    struct timeval start_time;
    struct timeval end_time;
    long x_elapsed_time;
    long f_elapsed_time;

    // This is necessary because xcode asks for permission the first
    // time it opens a file. Do it here so it doesn't mess up the timer.
    int fd = open(file_name, O_RDONLY);
    close(fd);
    
    // Test XOR
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < 100000; i++) {
        check_xor(file_name, 0);
    }
    gettimeofday(&end_time, NULL);
    x_elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    
    // Test Fletcher
    gettimeofday(&start_time, NULL);
    for (int i = 0; i < 100000; i++) {
        check_fletcher(file_name, 0);
    }
    gettimeofday(&end_time, NULL);
    f_elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);

    
    // Print results
    printf("XOR elapsed time: %li\n", x_elapsed_time);
    printf("Fletcher elapsed time: %li\n", f_elapsed_time);
}

/*
 Question 4 - skipped
 */

/*
 Question 5 - skipped
 */

