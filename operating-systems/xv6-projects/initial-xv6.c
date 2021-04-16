//
//  initial-xv6.c
//
//  Created by Annalise Tarhan on 4/16/21.
//

/*
 Implementing getreadcount() requires small changes across six files.
 
 The first step is to update the kernel to track the readcount, which
 is accomplished by adding a readcount variable to the cpu struct and
 then updating sys_read() to increment the cpu's readcount each time
 it is called. pushcli() and popcli() are used to disable interrupts
 in order to avoid deadlock while interacting with the cpu struct.
 
 Then, add a system function, sys_getreadcount(), to provide access to
 the cpu's readcount. The function itself goes in sysproc.c, but small
 additions to syscall.h, syscall.c, and usys.S are required for a user
 to be able to request that the kernel call the function on their behalf.
 */

/* proc.h */
struct cpu {
  // ...
  int readcount;
};

/* sysfile.c */
int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;
    
    /* Begin */
    pushcli();
    mycpu()->readcount++;
    popcli();
    /* End */
    
  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

/* sysproc.c */
int
sys_getreadcount(void)
{
    int readcount;
    pushcli();
    readcount = mycpu()->readcount;
    popcli();
    return readcount;
}

/* syscall.h */
#define SYS_getreadcount 22

/* syscall.c */
extern int sys_getreadcount(void);

static int (*syscalls[])(void) = {
    // ...
    [SYS_getreadcount]   sys_getreadcount,
};

/* usys.S */
SYSCALL(getreadcount)

