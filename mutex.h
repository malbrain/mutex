#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>

typedef union {
  struct {
	volatile uint16_t serving[1];
	volatile uint16_t next[1];
  };
  uint32_t bits;
} Ticket;

void mutex_lock(volatile char* mutex);
void mutex_unlock(volatile char* mutex);
void ticket_lock(Ticket* ticket);
void ticket_unlock(Ticket* ticket);

#endif
