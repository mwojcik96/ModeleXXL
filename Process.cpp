//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <zconf.h>
#include "Process.h"

const int Process::DO_YOU_CREATE_A_COMPETITION = 1;

void *Process::askIfCompetitionIsHeld(void *ptr) {
    int size;
    int rank;
    int buf;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Sending messages to check whether any competitions is held.\n");
    for(int i = 0; i < size; i++) {
        printf("Sending messages to %d process to check whether any competitions is held.\n", i);
        if(i != rank) {
            MPI_Send(&DO_YOU_CREATE_A_COMPETITION, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
    MPI_Status status;
    for(int i = 0; i < size; i++) {
        if(i != rank) {
            MPI_Recv(&buf, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("%d received from %d, I am %d\n", buf, i, rank);
        }
    }
    return nullptr;
}

void Process::behaviour() {
    pthread_t threadA;
    pthread_create(&threadA, NULL, Process::askIfCompetitionIsHeld, NULL);
    sleep((unsigned int) (rank + 1));
    pthread_join(threadA,NULL);
}

string Process::generateRoll() {
    int randomNumber = rand()%100+1;
    if (randomNumber < 15) {
        return ORGANIZER;
    }
    return PARTICIPANT;
}

Process::Process() {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("rank: %d, size: %d\n", rank, size);
}
