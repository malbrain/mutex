#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	FREE = 0,
	LOCKED,
	CONTESTED
} MutexState;

typedef volatile union {
	char state[1];
} CAS;

typedef volatile union {
	char state[1];
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

void cas_lock(CAS *cas);
void cas_unlock(CAS *cas);
void mcs_lock(MCS **mutex, MCS *qnode);
void mcs_unlock(MCS **mutex, MCS *qnode);
void mutex_lock(Mutex* mutex);
void mutex_unlock(Mutex* mutex);
void ticket_lock(Ticket* ticket);
void ticket_unlock(Ticket* ticket);

#endif
