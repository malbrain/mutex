# mutex
Compact spin latch and fair ticket latch for linux &amp; windows

The spin latch uses one byte for the latch and is obtained and released by:

    mutex_lock(char *latch);
    mutex_unlock(char *latch);
  
The fair ticket latch uses two 32bit words and is obtained and released by:

    ticket_lock(Ticket *ticket);
    ticket_unlock(Ticket *ticket);
  
  The Ticket structure is defined as:
  
    typedef struct {
      uint32_t serving[1];
      uint32_t next[1];
    } Ticket;

Initialize all latches to all-bits zero.  Both types utilize exponential back-off during latch contention.
