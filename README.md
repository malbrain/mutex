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

    typedef volatile union {
    	char state[1];
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
    (or cl /Ox /D STANDALONE mutex.c)

    ./mutex <# threads> <mutex type> ...

	x64/release/mutex
	Usage: x64/release/mutex #threads #type ...

Usage: mutex.exe #threads #type ...
0: System Type 40 bytes
1: Mutex Type 1 bytes
2: Ticket Type 4 bytes
3: MCS Type 16 bytes
4: CAS Type 1 bytes
5: FAA64 type 8 bytes
6: FAA32 type 4 bytes
7: FAA16 type 2 bytes


C:\Users\Owner\Source\Repos\malbrain\mutex>x64\release\mutex.exe 1 0
 real 14ns
 user 14ns
 sys  0ns
 nanosleeps 0

C:\Users\Owner\Source\Repos\malbrain\mutex>x64\release\mutex.exe 1 1
 real 7ns
 user 7ns
 sys  0ns
 nanosleeps 0

C:\Users\Owner\Source\Repos\malbrain\mutex>x64\release\mutex.exe 4 1
 real 7ns
 user 16ns
 sys  8ns
 nanosleeps 3666

C:\Users\Owner\Source\Repos\malbrain\mutex>x64\release\mutex.exe 4 0
 real 85ns
 user 330ns
 sys  0ns
 nanosleeps 0

C:\Users\Owner\Source\Repos\malbrain\mutex>x64\release\mutex.exe 4 4
 real 17ns
 user 62ns
 sys  0ns
 nanosleeps 0


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
