//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <unistd.h>
#include "Process.h"
#include "Agent.h"

const int Process::DO_YOU_CREATE_A_COMPETITION = 1;

void *Process::askIfCompetitionIsHeld(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    Agent agent;
    int buf;
    printf("Sending messages to check whether any competitions is held.\n");
    printf("FROM_STRUCT: size=%d, rank=%d, city=%d\n", sharedData->size, sharedData->rank, sharedData->city);
    for(int i = 0; i < sharedData->size; i++) {
        if(i != sharedData->rank) {
            printf("Sending messages to %d process to check whether any competitions is held. I am %d\n", i, sharedData->rank);
            MPI_Send(&DO_YOU_CREATE_A_COMPETITION, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
    MPI_Status status;
    for(int i = 0; i < sharedData->size-1; i++) {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_SOURCE != sharedData->rank) {
            agent.setGeneratingRole(false);
            if(buf == DO_YOU_CREATE_A_COMPETITION) {
                // send your role
                agent.setGeneratingRole(true);
                printf("%s jestem\n", agent.getRole().c_str());
            }
            printf("%d received from %d, I am %d\n", buf, status.MPI_SOURCE, sharedData->rank);
        }
    }
    if(agent.getGeneratingRole()) {
        printf("losuj");
        agent.setRole(agent.generateRoll());
    } else {
        agent.setRole(ORGANIZER);
    }
    printf("I will be %s\n", agent.getRole().c_str());
    return nullptr;
}

void Process::behaviour() {
    pthread_t threadA;
    pthread_create(&threadA, NULL, Process::askIfCompetitionIsHeld, &str);
    sleep((unsigned int) (str.rank + 1));
    pthread_join(threadA,NULL);
}



Process::Process() {
    MPI_Comm_rank(MPI_COMM_WORLD, &str.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &str.size);
    printf("rank: %d, size: %d\n", str.rank, str.size);
    str.city = str.size*str.rank;
    srand((unsigned int) time(NULL) + str.rank*20);
}
