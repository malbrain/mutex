//  mutex and ticket lock implmentation

//  please report bugs located to the program author,
//  malbrain@cal.berkeley.edu

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "mutex.h"

#ifdef linux
#include <linux/futex.h>
#include <limits.h>

#define SYS_futex 202
int sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3)
{
	return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

#endif

#ifdef unix
#define pause() asm volatile("pause\n": : : "memory")

void mutex_spin (int *cnt) {
struct timespec ts[1];
volatile int idx;

	if (*cnt < 8192)
	  *cnt += *cnt / 8;

	ts->tv_sec = 0;
	ts->tv_nsec = *cnt;

	if (*cnt < 1024 )
	  for (idx = 0; idx < *cnt; idx++)
		pause();
	else if (*cnt < 8192)
		sched_yield();
	else
		nanosleep(ts, NULL);
}

void mutex_lock(volatile char* mutex) {
uint32_t spinCount = 16;

  while (__sync_fetch_and_or(mutex, 1) & 1)
	while (*mutex & 1)
	  mutex_spin (&spinCount);
}

void mutex_unlock(volatile char* mutex) {
	asm volatile ("" ::: "memory");
	*mutex = 0;
}

void ticket_lock(Ticket* ticket) {
uint32_t spinCount = 16;
uint16_t ours;

	ours = __sync_fetch_and_add(ticket->next, 1);

	while (ours != ticket->serving[0])
	  mutex_spin (&spinCount);
}

void ticket_unlock(Ticket* ticket) {
	ticket->serving[0]++;
}

#else

// Replacement for nanosleep on Windows.

void nanosleep (const uint32_t ns, HANDLE *timer)
{
	LARGE_INTEGER sleepTime;

	sleepTime.QuadPart = ns / 100;

	if (!*timer)
		*timer = CreateWaitableTimer (NULL, TRUE, NULL);

	SetWaitableTimer (*timer, &sleepTime, 0, NULL, NULL, 0);
	WaitForSingleObject (*timer, INFINITE);
}

void mutex_spin (uint32_t *cnt, HANDLE *timer) {
volatile int idx;

	if (*cnt < 8192)
	  *cnt += *cnt / 8;

	if (*cnt < 1024 )
	  for (idx = 0; idx < *cnt; idx++)
		YieldProcessor();
	else if (*cnt < 8192)
		SwitchToThread();
	else
		nanosleep(*cnt, timer);
}

void mutex_lock(volatile char* mutex) {
uint32_t spinCount = 16;
HANDLE timer = NULL;

  while (_InterlockedOr8(mutex, 1) & 1)
	while (*mutex & 1)
	  mutex_spin(&spinCount, &timer);

  if (timer)
	CloseHandle(timer);
}

void mutex_unlock(volatile char* mutex) {
	*mutex = 0;
}

void ticket_lock(Ticket* ticket) {
uint32_t spinCount = 16;
HANDLE timer = NULL;
uint16_t ours;

	ours = (uint16_t)_InterlockedIncrement16(ticket->next) - 1;

	while( ours != ticket->serving[0] )
	  mutex_spin(&spinCount, &timer);

	if (timer)
		CloseHandle(timer);
}

void ticket_unlock(Ticket* ticket) {
	_InterlockedIncrement16(ticket->serving);
}
#endif

#ifdef STANDALONE
#ifdef unix

unsigned char Array[256] __attribute__((aligned(64)));
volatile char mutex[1] __attribute__((aligned(64)));
Ticket ticket[1] __attribute__((aligned(64)));
#else
__declspec(align(64)) unsigned char Array[256];
__declspec(align(64)) Ticket ticket[1];
__declspec(align(64)) char mutex[1];
#endif

enum {
	MutexType,
	TicketType
} lockType;

#ifndef unix
double getCpuTime(int type)
{
FILETIME crtime[1];
FILETIME xittime[1];
FILETIME systime[1];
FILETIME usrtime[1];
SYSTEMTIME timeconv[1];
double ans = 0;

	memset (timeconv, 0, sizeof(SYSTEMTIME));

	switch( type ) {
	case 0:
		GetSystemTimeAsFileTime (xittime);
		FileTimeToSystemTime (xittime, timeconv);
		ans = (double)timeconv->wDayOfWeek * 3600 * 24;
		break;
	case 1:
		GetProcessTimes (GetCurrentProcess(), crtime, xittime, systime, usrtime);
		FileTimeToSystemTime (usrtime, timeconv);
		break;
	case 2:
		GetProcessTimes (GetCurrentProcess(), crtime, xittime, systime, usrtime);
		FileTimeToSystemTime (systime, timeconv);
		break;
	}

	ans += (double)timeconv->wHour * 3600;
	ans += (double)timeconv->wMinute * 60;
	ans += (double)timeconv->wSecond;
	ans += (double)timeconv->wMilliseconds / 1000;
	return ans;
}
#else
#include <time.h>
#include <sys/resource.h>

double getCpuTime(int type)
{
struct rusage used[1];
struct timeval tv[1];

	switch( type ) {
	case 0:
		gettimeofday(tv, NULL);
		return (double)tv->tv_sec + (double)tv->tv_usec / 1000000;

	case 1:
		getrusage(RUSAGE_SELF, used);
		return (double)used->ru_utime.tv_sec + (double)used->ru_utime.tv_usec / 1000000;

	case 2:
		getrusage(RUSAGE_SELF, used);
		return (double)used->ru_stime.tv_sec + (double)used->ru_stime.tv_usec / 1000000;
	}

	return 0;
}
#endif

#ifdef unix
void *testit (void *arg) {
#else
UINT __stdcall testit (void *arg) {
#endif
uint64_t threadno = (uint64_t)arg;
int idx, first, loop;

	for (loop = 0; loop < 100000000; loop++) {
	  if (lockType == MutexType)
		mutex_lock(mutex);
	  else
		ticket_lock(ticket);

		first = Array[0];

#ifdef DEBUG
		if (!(loop % 10000))
			fprintf(stderr, "thread %lld loop %d\n", threadno, loop);
#endif

		for (idx = 0; idx < 255; idx++)
			Array[idx] = Array[idx + 1];

		Array[255] = first;

	  if (lockType == MutexType)
		mutex_unlock(mutex);
	  else
		ticket_unlock(ticket);
	}

#ifdef DEBUG
	fprintf(stderr, "thread %lld exiting\n", threadno);
#endif
	return 0;
}

int main (int argc, char **argv)
{
double start, elapsed;
uint64_t idx, cnt = 4;
#ifdef unix
pthread_t *threads;
#else
HANDLE *threads;
#endif

	start = getCpuTime(0);

	for (idx = 0; idx < 256; idx++)
		Array[idx] = idx;

	if (argc > 1)
		cnt = atoi(argv[1]);

	if (argc > 2)
		lockType = atoi(argv[2]);

	if (lockType == MutexType)
		fprintf(stderr, "Mutex Type\n");

	if (lockType == TicketType)
		fprintf(stderr, "Ticket Type\n");

#ifdef unix
	threads = malloc (cnt * sizeof(pthread_t));
#else
	threads = GlobalAlloc (GMEM_FIXED|GMEM_ZEROINIT, cnt * sizeof(HANDLE));
#endif

	for (idx = 0; idx < cnt; idx++) {
#ifdef unix
	  if( pthread_create (threads + idx, NULL, testit, idx) )
		fprintf(stderr, "Unable to create thread, errno = %d\n", errno);
#else
	  threads[idx] = (HANDLE)_beginthreadex(NULL, 131072, testit, (void *)idx, 0, NULL);
#endif
	}

	// 	wait for termination

#ifdef unix
	for( idx = 0; idx < cnt; idx++ )
		pthread_join (threads[idx], NULL);
#else
	WaitForMultipleObjects (cnt, threads, TRUE, INFINITE);

	for( idx = 0; idx < cnt; idx++ )
		CloseHandle(threads[idx]);
#endif

	for( idx = 0; idx < 256; idx++)
	  if (Array[idx] != (unsigned char)(Array[(idx+1) % 256] - 1))
		fprintf (stderr, "Array out of order\n");

	elapsed = getCpuTime(0) - start;
	fprintf(stderr, " real %.0fns\n", elapsed * 10);
	elapsed = getCpuTime(1);
	fprintf(stderr, " user %.0fns\n", elapsed * 10);
	elapsed = getCpuTime(2);
	fprintf(stderr, " sys  %.0fns\n", elapsed * 10);
}
#endif
