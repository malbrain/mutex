# mutex
Compact mutex latch and ticket latch for linux &amp; windows

The mutex latch uses one byte for the latch and is obtained and released by:

    mutex_lock(Mutex *latch);
    mutex_unlock(Mutex *latch);

  The Mutex structure is defined as:

    typedef union {
    #ifdef FUTEX
      struct {
        volatile uint16_t lock[1];
        uint16_t futex;
      };
      uint32_t bits[1];
    #else
      volatile char lock[1];
    #endif
    } Mutex;
  
The ticket latch uses two 16 bit shorts and is obtained and released by:

    ticket_lock(Ticket *ticket);
    ticket_unlock(Ticket *ticket);
  
  The Ticket structure is defined as:
  
    typedef union {
	  struct {
      	volatile uint16_t serving[1];
      	volatile uint16_t next[1];
	  };
	  uint32_t bits;
    } Ticket;

Initialize all latches to all-bits zero.  Both latch types utilize exponential back-off during latch contention.
