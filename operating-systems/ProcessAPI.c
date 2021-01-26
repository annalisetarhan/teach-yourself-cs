//
//  ProcessAPI.c
//  OSTEP
//
//  Created by Annalise Tarhan on 1/26/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

/*
 When both parent and child processes change an apparently shared variable,
 they are actually changing different copies of that variable.
 */
void q1() {
    int x = 100;
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        printf("Child says: x is %i\n", x);
        x += 5;
        printf("Child says: now x is %i\n", x);
    } else {
        printf("Parent says: x is %i\n", x);
        x += 10;
        printf("Parent says: now x is %i\n", x);
    }
}

/*
 Yes, both parent and child can access the file descriptor.
 If they write concurrently, one overwrites the other.
 */
void q2() {
    int fd = open("q2.txt", O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
    assert(fd > -1);
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        long child_rc = write(fd, "Child says hello\n", 19);
        assert(child_rc == 19);
        close(fd);
    } else {
        long parent_rc = write(fd, "Parent says hello\n", 20);
        assert(parent_rc == 20);
        close(fd);
    }
}

/*
 This doesn't quite guarantee that the child will run before the parent,
 but since the file write in the parent is an I/O operation, it's very
 likely that the child will finish before the parent does.
 */
void q3() {
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        printf("hello\n");
    } else {
        int fd = open("dummy.txt", O_CREAT, S_IRWXU);
        write(fd, "This should take a while\n", 27);
        close(fd);
        printf("goodbye\n");
    }
}

/*
 Each version of exec offers a different combination of options:
 whether the program is given as part of a fully qualified path or
 just as the file name, whether the arguments are given in a null
 terminated list or as an array, and whether or not environment
 variables are included.
 */
void q4() {
    const char *file = "ls";
    const char *path = "/bin/ls";
    const char *arg0 = "ls";
    const char *arg1 = "-a";
    char *null_term = 0;
    char *const args[] = {"ls", "-s", NULL};
    char *const envp[] = {"LS_COLORS=di=1;31", NULL};
    
    int rc = fork();
    if (rc == 0) {
        execl(path, arg0, arg1, null_term);
    }
    rc = fork();
    if (rc == 0) {
        execle(path, arg0, arg1, null_term, envp);
    }
    rc = fork();
    if (rc == 0) {
        execlp(file, arg0, arg1, null_term);
    }
    rc = fork();
    if (rc == 0) {
        execv(path, args);
    }
    rc = fork();
    if (rc == 0) {
        execvp(file, args);
    }
}

/*
 In the parent, wait() returns the pid of the child process.
 In the child, wait() returns -1, indicating an error. The
 error code is 10, "No child processes."

 Note: On macOS, wait() is consistently interrupted by the system,
 so it returns -1, even in the parent. In order to get a useful
 result, use a loop and wait for the child process pid or an actual
 error.
 */
void q5() {
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        while (1) {
            int child_wait = wait(NULL);
            if (child_wait == -1 && errno == EINTR) {
                continue;
            } else if (child_wait > 0) {
                printf("(Grand)Child pid is %i\n", child_wait);
                break;
            } else {
                printf("Actual error: %i - %s\n", errno, strerror(errno));
                break;
            }
        }
    } else {
        while (1) {
            int parent_wait = wait(NULL);
            if (parent_wait == -1 && errno == EINTR) {
                continue;
            } else if (parent_wait > 0) {
                printf("Child pid is %i\n", parent_wait);
                break;
            } else {
                printf("Actual error: %i - %s\n", errno, strerror(errno));
                break;
            }
        }
    }
}

/*
 To use waitpid, replace wait(NULL) with waitpid(-1, NULL, 0).
 The first argument, -1, indicates that waitpid should wait for
 any child process, which is the default behavior for wait().
 The second is an address to store status information (which is
 the same as the NULL in wait()). The third argument is an int
 that can be used for options flags.   `
 */
void q6() {
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        while (1) {
            int child_wait = waitpid(-1, NULL, 0);
            if (child_wait == -1 && errno == EINTR) {
                continue;
            } else if (child_wait > 0) {
                printf("(Grand)Child pid is %i\n", child_wait);
                break;
            } else {
                printf("Actual error: %i - %s\n", errno, strerror(errno));
                break;
            }
        }
    } else {
        while (1) {
            int parent_wait = waitpid(-1, NULL, 0);
            if (parent_wait == -1 && errno == EINTR) {
                continue;
            } else if (parent_wait > 0) {
                printf("Child pid is %i\n", parent_wait);
                break;
            } else {
                printf("Actual error: %i - %s\n", errno, strerror(errno));
                break;
            }
        }
    }
}

/*
 If the child closes standard output, it will no longer be able to print
 but the parent process still will.
 */
void q7() {
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork failed\n");
    } else if (rc == 0) {
        close(STDOUT_FILENO);
        printf("Child: Hello? Stdout?\n");
    } else {
        while (1) {
            int pid = waitpid(-1, NULL, 0);
            if (pid == -1 && errno == EINTR) {
                continue;
            } else if (pid == -1) {
                printf("Actual error: %i - %s\n", errno, strerror(errno));
                break;
            } else {
                printf("Parent: Hello? Stdout?\n");
                break;
            }
        }
    }
}

/*
 How cool is this? The parent process creates the pipe and
 the two child processes inherit the file descriptors, allowing
 one to talk to the other.
 */
void q8() {
    int pipefd[2];
    char buf;
    if (pipe(pipefd) == -1) {
        fprintf(stderr, "Pipe failed\n");
    }
    int rc = fork();
    /* The first child reads */
    if (rc == 0) {
        close(pipefd[1]);
        while (read(pipefd[0], &buf, 1) > 0) {
            write(STDOUT_FILENO, &buf, 1);
        }
        close(pipefd[0]);
    }
    rc = fork();
    /* The second child writes */
    if (rc == 0) {
        close(pipefd[0]);
        write(pipefd[1], "Hello, hello! Anyone there?\n", 28);
        close(pipefd[1]);
    }
}

int main(int argc, const char * argv[]) {
    q1();
//    q2();
//    q3();
//    q4();
//    q5();
//    q6();
//    q7();
//    q8();
    return 0;
}

