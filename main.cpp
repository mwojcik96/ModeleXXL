#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>
#include "Process.h"

/*
    Program demonstruje bezpieczne (w miarę) użycie wštków w MPI

    Przesyłana jest liczba w piercieniu.
*/

void *startFunc(void *ptr)
{
    MPI_Status status;
    int size;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("SIZE: %d\n", size);


    int x=1;
    int data=0;
    while (x) {
        x--;
        printf("\t%d czeka\n", rank);

        if ((rank == 0) && (x==0)) {
            printf("0 Będę spał\n");
            sleep (10);
            printf("0 kończy spanie\n");
        }

        MPI_Recv( &data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("%d dostal %d od %d\n", rank, data, status.MPI_SOURCE);

        data++;
        if (data>=size) {
            printf("[%d] Pierwszy koniec dla %d\n", rank, data);
        } else if (rank==size-1) {
            printf("%d wysyla %d do 0\n", rank, data);
            /* MPI_Ssend teoretycznie powinien się blokować aż do matching receive
                (tak twierdzi kilka wyguglanych stron)
            */

            MPI_Ssend( &data, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("%d wyslanie %d do 0 się powiodło \n", rank, data);
        } else {
            printf("%d wysyla %d do %d\n", rank, data, rank+1);
            MPI_Ssend( &data, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
        }
    }
    return nullptr;
}

int main(int argc,char **argv)
{
//    system("pkill -f PRy");
    int provided=0;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    printf("THREAD SUPPORT: %d\n", provided);
    printf("%s, %s\n", argv[1], argv[2]);
    if (provided!=MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "Brak wystarczajacego wsparcia dla watkow - wychodze!\n");
        MPI_Finalize();
        exit(-1);
    }

    int size;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Process *process = new Process(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), strtol(argv[1], NULL, 10));
    process->behaviour();
//    pthread_t threadA;
//    pthread_create( &threadA, NULL, askIfCompetitionIsHeld, 0);
//
//    int data=0;
//
//    sleep((unsigned int) (rank + 1));
//
//    /*   if (rank==size-1)
//       MPI_Ssend( &data, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
//       else */
//    if (rank==0)
//        MPI_Ssend( &data, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
//
//    printf("--> [%d] Poszlo! %d \n", rank, data);
//
//    pthread_join(threadA,NULL);

    printf("    %d koniec!\n", rank);
    MPI_Finalize();
}