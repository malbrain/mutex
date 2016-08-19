//  mutex and ticket lock implmentation

//  please report bugs located to the program author,
//  malbrain@cal.berkeley.edu

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "mutex.h"

#ifdef unix
#define pause() asm volatile("pause\n": : : "memory")

void mutex_lock(volatile char* mutex) {
int spinCount = 16;
int prev, idx;

  do {
	if(*mutex & 1)
		prev = 1;
	else
		prev = __sync_fetch_and_or(mutex, 1);

	if( ~prev & 1 )
		return;

	for (idx = 0; idx < spinCount; idx++)
		pause();

	if (spinCount < 16*1024*1024)
		spinCount *= 2;
  } while( 1 );
}

void mutex_unlock(char* mutex) {
	*mutex = 0;
}

void ticket_lock(Ticket* ticket) {
int idx, spinCount = 16;
uint32_t ours;

	ours = __sync_fetch_and_add(ticket->next, 1);

	while( ours != *ticket->serving ) {

		for (idx = 0; idx < spinCount; idx++) {
			pause();
		}

		if (spinCount < 16*1024*1024) {
			spinCount *= 2;
		}
	}
}

void ticket_unlock(Ticket* ticket) {
	__sync_fetch_and_add(ticket->serving, 1);
}
#else

void mutex_lock(volatile char* mutex) {
int spinCount = 16;
int prev, idx;

  do {
	//  see if we can get write access
	//	with no readers

	if(*mutex & 1)
		prev = 1;
	else
		prev = _InterlockedOr8(mutex, 1);

	if( ~prev & 1 )
		return;

	for (idx = 0; idx < spinCount; idx++)
		YieldProcessor();

	if (spinCount < 16*1024*1024)
		spinCount *= 2;
  } while( 1 );
}

void mutex_unlock(char* mutex) {
	*mutex = 0;
}

void ticket_lock(Ticket* ticket) {
int idx, spinCount = 16;
uint32_t ours;

	ours = _InterlockedIncrement(ticket->next) - 1;

	while( ours != *ticket->serving ) {
		for (idx = 0; idx < spinCount; idx++)
			YieldProcessor();

		if (spinCount < 16*1024*1024)
			spinCount *= 2;
	}
}

void ticket_unlock(Ticket* ticket) {
	_InterlockedIncrement(ticket->serving);
}
#endif
