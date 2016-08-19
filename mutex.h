#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>

typedef struct {
	uint32_t serving[1];
	uint32_t next[1];
} Ticket;

void mutex_lock(char* mutex);
void mutex_unlock(char* mutex);
void ticket_lock(Ticket* ticket);
void ticket_unlock(Ticket* ticket);

#endif
