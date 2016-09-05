# mutex
Compact mutex latch and ticket latch for linux &amp; windows

The mutex latch uses one byte for the latch and is obtained and released by:

    mutex_lock(Mutex *latch);
    mutex_unlock(Mutex *latch);

The Mutex structure is defined as:

    typedef enum {
    	FREE = 0,
    	LOCKED,
    	CONTESTED
    } MutexState;

    typedef volatile struct {
    #ifdef FUTEX
    	MutexState state[1];
    #else
    	char lock[1];
    #endif
    } Mutex;

The ticket latch uses two 16 bit shorts and is obtained and released by:

    ticket_lock(Ticket *ticket);
    ticket_unlock(Ticket *ticket);
  
The Ticket structure is defined as:
  
    typedef struct {
      volatile uint16_t serving[1];
      volatile uint16_t next[1];
    } Ticket;

Initialize all latches to all-bits zero.  All latch types utilize exponential back-off during latch contention.

Compile with -D FUTEX to use futex contention on linux.

Define STANDLONE during compilation to perform basic benchmarks on your system:

    gcc -o mutex -g -O3 -D STANDALONE mutex.c -lpthread
    (or cl /Ox /D STANDAONE mutex.c)

    ./mutex <# threads> <mutex type>

    Usage: ./mutex #threads #type
    0: System Type 40 bytes
    1: Mutex Type 4 bytes
    2: Ticket Type 4 bytes
    3: MCS Type 16 bytes

Sample linux 64 bit output (non-FUTEX):


    [root@test7x64 xlink]# ./mutex 2 1
     real 42ns
     user 85ns
     sys  0ns
     nanosleeps 84

    [root@test7x64 xlink]# ./mutex 20 1
     real 34ns
     user 93ns
     sys  1ns
     nanosleeps 347934

    [root@test7x64 xlink]# ./mutex 200 1
     real 36ns
     user 132ns
     sys  8ns
     nanosleeps 938255

    [root@test7x64 xlink]# ./mutex 2000 1
     real 35ns
     user 131ns
     sys  9ns
     nanosleeps 921295

Sample linux output (FUTEX):

    [root@test7x64 xlink]# cc -o mutex -g -O3 -D STANDALONE -D FUTEX mutex.c -lpthread

    [root@test7x64 xlink]# ./mutex 2 1
     real 66ns
     user 132ns
     sys  0ns
     futex waits: 3
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 20 1
     real 120ns
     user 475ns
     sys  0ns
     futex waits: 22728
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 200 1
     real 119ns
     user 471ns
     sys  1ns
     futex waits: 38041
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 2000 1
     real 121ns
     user 478ns
     sys  2ns
     futex waits: 49069
     nanosleeps 0

Please address any questions or concerns to the program author: Karl Malbrain, malbrain@cal.berkeley.edu.
