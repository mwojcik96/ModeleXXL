//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include "Process.h"
#include "Agent.h"
#include <syscall.h>

using namespace std;

#define PRESTATE                    1000       //Przed zadaniem pytań
#define ASK_ORGANIZATION            1001       //Po zadaniu pytań(100) - czeka na odpowiedzi(200) i po nich decyduje kim jest (wysyła (300))

#define DECIDED_TO_PARTICIPATE      10      //Uczestnik - czeka na (800) przed spytaniem o hotel(400)
#define ASK_HOTEL                   11      //Uczestnik - czekający na hotel i po zgodzie((500) * n-H) wysyłający potwierdzenie do organizatora(900)
#define WAITING_FOR_END             12      //Uczestnik - czekający na info o zakończeniu konkrsu(1000)
#define AFTER_COMPETITION_IN_HOTEL  13      //Uczestnik - po konkursie w czasie gdy jeszcze okupuje hotel - potem rozsyła zgody czekającym na hotel(500)

#define DECIDED_TO_ORGANIZE         20      //Organizator - przed spytaniem o salę(600) (może niepotrzebny?)
#define ASK_HALL                    21      //Organizator - czekający na salę i po zgodzie((700) * n-1) wysyła H zaproszeń(type 200)
#define ASK_INVITES                 22      //Organizator - czeka na odpowiedzi(300) aż lista potencjalnych będzie pusta; wtedy rozsyła(800) PS. TU WĄTEK DO ODBIERANIA (100) zmienia strategię
#define RECV_HOTEL_RESERVATIONS     23      //Organizator - czeka na info o rezerwacjach ((900) * C), rozsyła koniec konkursu(1000) i zgodę czekającym na salę(700)

#define COMPETITION_QUESTION        100     //"Czy organizujesz konkurs?"
#define COMPETITION_ANSWER          200     //"Organizuję konkurs w M"
#define COMPETITION_CONFIRM         300     //"Będę/Nie będę"

#define HOTEL_QUESTION              400     //"Czy mogę wziąć hotel w M?"
#define HOTEL_ANSWER                500     //"Możesz wziąć hotel w M"

#define HALL_QUESTION               600     //"Czy mogę wziąć S w M?"
#define HALL_ANSWER                 700     //"Możesz wziąć S w M"

#define SIGN_IN_END                 800     //"Zapisy na konkurs w M się skończyły"
#define HOTEL_BOOKED                900     //"Mam hotel w M"
#define COMPETITION_END             1000    //"Koniec konkursu"

const int Process::DO_YOU_CREATE_A_COMPETITION = 1;
// HERE'S THE DEFINITION OF MUTEXES
pthread_mutex_t mutex1[10];

void incrementAfterMPISend(structToSend ourStr) {
    ourStr.clock[ourStr.rank]++;
}
// returns -1 when second argument clock is greater than first, 0 when they are equal, 1 when first is greater
// and 100 when they are incomparable
int compareClocks(vector<int> ourClock, vector<int> receivedClock) {
    int output = 0;
    for(int i = 0; i < ourClock.size(); i++) {
        if(ourClock[i] < receivedClock[i]) {
            if (!output)
                output = -1;
            else if (output == 1) {
                output = 100;
                break;
            } else
                continue;
        }
        else if (ourClock[i] == receivedClock[i]){
            continue;
        } else {
            if(!output)
                output = 1;
            else if (output == 1) {
                output = 100;
                break;
            }
        }
    }
    return output;
}

void incrementAfterMPIRecv(structToSend ourStr, vector<int> receivedClock) {
    incrementAfterMPISend(ourStr);
    for(int i = 0; i < receivedClock.size(); i++) {
        ourStr.clock[i] = ourStr.clock[i] >= receivedClock[i] ? ourStr.clock[i] : receivedClock[i];
    }
}



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
                printf("%d jestem\n", agent.getRole());
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
    printf("I will be %d\n", agent.getRole());
    return nullptr;
}
void Process::sendMessagesAskingIfCompetitionIsHeld(structToSend str) {
    for(int i = 0; i < str.size; i++) {
        if(i != str.rank) {
            printf("Sending messages to %d process to check whether any competitions is held. I am %d\n", i, str.rank);
            MPI_Send(&DO_YOU_CREATE_A_COMPETITION, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
}
void Process::behaviour() { // sendy
    printf("Sending messages to check whether any competitions is held.\n");
    printf("FROM_STRUCT: size=%d, rank=%d, city=%d\n", str.size, str.rank, str.city);
    sendMessagesAskingIfCompetitionIsHeld(str);
    pthread_t threadA;
    pthread_t threadB;
    pthread_create(&threadA, NULL, Process::organizationResponder, &str);
    pthread_create(&threadB, NULL, Process::organizationResponder, &str);
    sleep((unsigned int) (str.rank + 1));
    pthread_join(threadA,NULL);
    pthread_join(threadB,NULL);
}

void* Process::organizationResponder(void *ptr) {
    printf("creating xD \n");
    structToSend *sharedData = (structToSend *) ptr;
    int buf = 1;
    int decision;
    MPI_Status status;
    // DOESN'T WORK HERE
    pthread_mutex_lock(&mutex1[0]);
    printf("INSIDE: tid = %d, mutex_addr=%d, process = %d\n", syscall(SYS_gettid), &mutex1[0], sharedData->rank);
    sleep(3);
    printf("LEFT: tid = %d, mutex_addr=%d, process = %d\n", syscall(SYS_gettid), &mutex1[0], sharedData->rank);
    pthread_mutex_unlock(&mutex1[0]);
    /*
    while(true){
        // to disable CLion's verification of endless loop
        if(sharedData->state == 2000000)
            break;
        decision = -1; //default_value = "NO"
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, COMPETITION_QUESTION, MPI_COMM_WORLD, &status);
        // MUTEXES
        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        pthread_mutex_lock(&mutex1[POTENTIAL_USERS_MUTEX]);
        pthread_mutex_lock(&mutex1[SIGNED_USERS_MUTEX]);
        //NOTE: city and hotelSlots mutex not needed; main thread can't change it in this state
        if(sharedData->state == ASK_INVITES && freeSlotInVectors(sharedData)) {
            decision = sharedData->city;
            sharedData->potentialUsers.push_back(status.MPI_SOURCE);
        }
        MPI_Send(&decision, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
        pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
        pthread_mutex_unlock(&mutex1[POTENTIAL_USERS_MUTEX]);
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
        //here mutex_signedUsers(unlock)
        //here mutex_potentialUsers(unlock)
        //here mutex_state(unlock)
    } */
    return nullptr;
}

bool Process::freeSlotInVectors(structToSend* str) {
    return(str->potentialUsers.size() + str->signedUsers.size() >= str->hotelSlots);
}

Process::Process() {
    MPI_Comm_rank(MPI_COMM_WORLD, &str.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &str.size);
    printf("rank: %d, size: %d\n", str.rank, str.size);
    str.city = -1;
    str.hall = -1;
    str.state = 0;
    str.hotelSlots = 30;
    srand((unsigned int) time(NULL) + str.rank*20);
}
