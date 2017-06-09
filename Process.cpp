//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <zconf.h>
#include "Process.h"
#include "Agent.h"

const int Process::DO_YOU_CREATE_A_COMPETITION = 1;

void *Process::askIfCompetitionIsHeld(void *ptr) {
    Agent agent;
    int size;
    int rank;
    int buf;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Sending messages to check whether any competitions is held.\n");
    for(int i = 0; i < size; i++) {
        if(i != rank) {
            printf("Sending messages to %d process to check whether any competitions is held. I am %d\n", i, rank);
            MPI_Send(&DO_YOU_CREATE_A_COMPETITION, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
    MPI_Status status;
    for(int i = 0; i < size-1; i++) {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_SOURCE != rank) {
            agent.setGeneratingRole(false);
            if(buf == DO_YOU_CREATE_A_COMPETITION) {
                // send your role
                agent.setGeneratingRole(true);
                printf("%d jestem\n", agent.getRole());
            }
            printf("%d received from %d, I am %d\n", buf, status.MPI_SOURCE, rank);
        }
    }
    if(agent.getGeneratingRole()) {
        printf("losuj");
        agent.setRole(agent.generateRoll());
    } else {
        agent.setRole(ORGANIZER);
    }
    printf("I will be %d\n", agent.getRole());
    return nullptr;
}

void Process::behaviour() { // sendy
    pthread_t threadA;
    pthread_create(&threadA, NULL, Process::askIfCompetitionIsHeld, NULL);
    sleep((unsigned int) (rank + 1));
    pthread_join(threadA,NULL);
}



Process::Process() {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("rank: %d, size: %d\n", rank, size);
    srand((unsigned int) time(NULL) + rank*20);
}
