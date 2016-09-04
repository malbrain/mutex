#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>

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

typedef struct {
	volatile uint16_t serving[1];
	volatile uint16_t next[1];
} Ticket;

typedef struct {
  void * volatile next;
#ifdef _WIN32
  HANDLE wait;
#else
#ifdef FUTEX
  union {
	struct {
	  volatile uint16_t lock[1];
	  uint16_t futex;
	};
	uint32_t bits[1];
  };
#else
  volatile char lock[1];
#endif
#endif
} MCS;

void mcs_lock(MCS **mutex, MCS *qnode);
void mcs_unlock(MCS **mutex, MCS *qnode);
void mutex_lock(Mutex* mutex);
void mutex_unlock(Mutex* mutex);
void ticket_lock(Ticket* ticket);
void ticket_unlock(Ticket* ticket);

#endif
