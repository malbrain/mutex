# mutex
Compact mutex latch and ticket latch for linux &amp; windows

The mutex latch uses one byte for the latch and is obtained and released by:

    mutex_lock(Mutex *latch);
    mutex_unlock(Mutex *latch);

The Mutex structure is defined as:

    typedef volatile union {
    #ifdef FUTEX
      struct {
        volatile uint16_t lock[1];
        uint16_t futex;
      };
      uint32_t bits[1];
    #else
      char lock[1];
    #endif
    } Mutex;
  
The Mutex2 structure is defined as:

    typedef volatile union {
    #ifdef FUTEX
      struct {
        uint16_t xcl[1];
        uint16_t waiters;
      };
      uint32_t value[1];
    #else
      char lock[1];
    #endif
    } Mutex2;
  
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

where <mutex type> is:
    0: System Type
    1: Mutex Type
    2: Mutex2 Type
    3: Ticket Type	(Non Scalable)
    4: MCS type		(Non Scalable)

Sample linux output (non-FUTEX):

    Mutex Type 1 bytes

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

    Mutex2 Type 1 bytes

    [root@test7x64 xlink]# ./mutex 2 2
     real 23ns
     user 26ns
     sys  0ns
     nanosleeps 35298

    [root@test7x64 xlink]# ./mutex 20 2
     real 24ns
     user 36ns
     sys  1ns
     nanosleeps 597483

    [root@test7x64 xlink]# ./mutex 200 2
     real 38ns
     user 60ns
     sys  72ns
     nanosleeps 6795226

    [root@test7x64 xlink]# ./mutex 2000 2
     real 39ns
     user 61ns
     sys  91ns
     nanosleeps 6978030

Sample linux output (FUTEX):

    Mutex Type 4 bytes

    [root@test7x64 xlink]# cc -o mutex -g -O3 -D STANDALONE -D FUTEX mutex.c -lpthread
    [root@test7x64 xlink]# ./mutex 2 1
     real 49ns
     user 99ns
     sys  0ns
     futex waits: 72
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 20 1
     real 113ns
     user 203ns
     sys  249ns
     futex waits: 645817
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 200 1
     real 117ns
     user 208ns
     sys  261ns
     futex waits: 663790
     nanosleeps 0

    [root@test7x64 xlink]# ./mutex 2000 1
     real 153ns
     user 217ns
     sys  396ns
     futex waits: 752642
     nanosleeps 0


