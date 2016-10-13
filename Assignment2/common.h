//Alexander Chatron-Michaud, 260611509
//COMP 310 Winter 2016
//shoutout to Jit for giving us a nice head start on this annoying assignment

#ifndef _INCLUDE_COMMON_H_
#define _INCLUDE_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MY_SHM 888

typedef struct jobtype {
	int id;
	int duration;
	int pagesToPrint;
} JobType;

typedef struct Queue {
	JobType elements[128];
	int front;
	int end;
	int size;
} Queue;

typedef struct shared_mem {
	Queue queue;
	sem_t mutex;
	sem_t empty;
	sem_t full;
} SharedMem;

void pushQueue(Queue* buffer, JobType toBeAdded) {
    buffer->elements[buffer->end] = toBeAdded;
    buffer->elements[buffer->end].duration = toBeAdded.duration;
    buffer->elements[buffer->end].pagesToPrint = toBeAdded.pagesToPrint;
    buffer->elements[buffer->end].id = toBeAdded.id;
    buffer->end = (buffer->end + 1) % buffer->size;
}

JobType popQueue(Queue* buffer) {
    JobType popped = buffer->elements[buffer->front];
    buffer->front = (buffer->front + 1) % buffer->size;
    return popped;
}

int isQueueEmpty(sem_t* full) {
	int fullVal;
	sem_getvalue(full, &fullVal);
	return (fullVal == 0);
}

#endif //_INCLUDE_COMMON_H_

