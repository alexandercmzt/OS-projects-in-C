//Alexander Chatron-Michaud, 260611509
//COMP 310 Winter 2016
//shoutout to Jit for giving us a nice head start on this annoying assignment

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int sharedMemNumber;
SharedMem* shared;

void performJob(JobType* job, int queueIndex) {
    printf("Printer starts printing %d pages from Buffer[%d]\n", job->pagesToPrint, queueIndex);
    fflush(stdout);
    sleep(job->duration);
    printf("Printer finishes printing %d pages from Buffer[%d]\n", job->pagesToPrint, queueIndex);
}

void setup_shared_mem() {
	sharedMemNumber = shmget(MY_SHM, sizeof(SharedMem), 0777 | IPC_CREAT);
}
void attach_shared_mem(int blength) {
	shared = (void *) shmat(sharedMemNumber, NULL, 0);
    Queue* queue = &shared->queue;
    queue->size = blength;
    queue->front = 0;
    queue->end = 0;
    int i;
    //fill the queue
    for (i = 0; i < 128; i++) {
        JobType* current;
        current = &shared->queue.elements[i];
        current->id = 0;
        current->duration = 0;
        current->pagesToPrint = 0;
    }
}

void init_semaphore(int blength) { //set up the semaphores we need
    sem_init(&shared->mutex,1,1);//used to make a critical section
	sem_init(&shared->empty,1,blength);//used to detect when queue is empty
    sem_init(&shared->full,1,0);//used to detect when queue is full
}

int take_a_job(JobType* job) {
    if (isQueueEmpty(&shared->full)) {
        printf("Printer is currently waiting for jobs.\n");
    }
    sem_wait(&shared->full);
    sem_wait(&shared->mutex); //we only go beyond this point if we're not full and this is the only thing printing
    int jobIndex = shared->queue.front;
    JobType sharedMemJob = popQueue(&shared->queue);
    *job = sharedMemJob;
    sem_post(&shared->mutex); //end of the critical section
    sem_post(&shared->empty);
    return jobIndex; //returns the index of which job was done
}

int main(int argc, char *argv[]) {
    if (argc < 2) { //simple argument checking
        printf("Please provide queue size as argument\n");
        exit(1);
    }
    int blength = atoi(argv[1]); //blength is the size of the queue we're going to use
    if (blength > 128 || blength < 1) {
        printf("Error: Buffer is either negative/zero size or greater than 128.\n");
        exit(1);
    } //I set the max at 128, feel free to change it
	setup_shared_mem();
	attach_shared_mem(blength);
	init_semaphore(blength);
    JobType job = {0, 0, 0}; //placeholder
	while (1) {
        int queueIndex;
		queueIndex = take_a_job(&job);
        performJob(&job, queueIndex);
	}
}
