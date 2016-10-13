//Alexander Chatron-Michaud, 260611509
//COMP 310 Winter 2016
//shoutout to Jit for giving us a nice head start on this annoying assignment

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

void release_share_mem(SharedMem* sharedMemory) {
    shmdt(sharedMemory);
}

SharedMem* attach_share_mem() {
	int sharedMemNumber = shmget(MY_SHM, sizeof(SharedMem), 0);
    if (sharedMemNumber == -1) {
        printf("shmget error or no printer is running\n");
        exit(1);
    }
    SharedMem* address = NULL;
	address = shmat(sharedMemNumber, address, 0);
    return address;
}

JobType create_job(int id, int pagesToPrint, int duration) {
    return (JobType) {id,duration,pagesToPrint};
}

void put_a_job(SharedMem* memory, JobType job) {
    int fullqueue = 0; //this is like a boolean, this is so we know if the client needs to wait for its turn yet
    if (isQueueEmpty(&memory->empty)) {
        printf("Client %d has %d pages to print, Printer buffer full, going to sleep\n", job.id, job.pagesToPrint);
        fflush(stdout);
        fullqueue = 1;
    }
    sem_wait(&memory->empty);
    sem_wait(&memory->mutex);
    if (fullqueue) { //check which message to use when putting a client into the buffer
        printf("Client %d is woken up and puts request in Buffer[%d]\n", job.id, memory->queue.end);
    } else {
        printf("Client %d has to print %d pages, puts request in Buffer[%d]\n", job.id, job.pagesToPrint, memory->queue.end);
    }
    pushQueue(&memory->queue, job);
    sem_post(&memory->mutex);
    sem_post(&memory->full);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: ./client id pagesToPrint duration\n");
        exit(1);
    }
    int clientId = atoi(argv[1]);
    int pagesToPrint = atoi(argv[2]);
    int duration = atoi(argv[3]);
	SharedMem* sharedMemory = attach_share_mem();
	JobType job = create_job(clientId, pagesToPrint, duration);
	put_a_job(sharedMemory, job);
	release_share_mem(sharedMemory);
}


