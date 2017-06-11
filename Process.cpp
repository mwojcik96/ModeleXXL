//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include "Process.h"
#include <algorithm>


using namespace std;

#define PRESTATE                    1000    //Przed zadaniem pytań
#define ASK_ORGANIZATION            1001    //Po zadaniu pytań(100) - czeka na odpowiedzi(200) i po nich decyduje kim jest (wysyła (300))

#define DECIDED_TO_PARTICIPATE      14      //Uczestnik - czeka na (800) przed spytaniem o hotel(400)
#define ASK_HOTEL                   15      //Uczestnik - czekający na hotel i po zgodzie((500) * n-H) wysyłający potwierdzenie do organizatora(900)
#define WAITING_FOR_END             16      //Uczestnik - czekający na info o zakończeniu konkrsu(1000)
#define AFTER_COMPETITION_IN_HOTEL  17      //Uczestnik - po konkursie w czasie gdy jeszcze okupuje hotel - potem rozsyła zgody czekającym na hotel(500)

#define DECIDED_TO_ORGANIZE         20      //Organizator - przed spytaniem o salę(600) (może niepotrzebny?)
#define ASK_HALL                    21      //Organizator - czekający na salę i po zgodzie((700) * n-1) wysyła H zaproszeń(type 200)
#define ASK_INVITES                 22      //Organizator - czeka na odpowiedzi(300) aż lista potencjalnych będzie pusta; wtedy rozsyła(800) PS. TU WĄTEK DO ODBIERANIA (100) zmienia strategię
#define RECV_HOTEL_RESERVATIONS     23      //Organizator - czeka na info o rezerwacjach ((900) * C), rozsyła koniec konkursu(1000) i zgodę czekającym na salę(700)

#define COMPETITION_QUESTION        100     //"Czy organizujesz konkurs?"
#define COMPETITION_ANSWER          200     //"Organizuję konkurs w M"
#define COMPETITION_CONFIRM         300     //"Będę"/"Nie będę"

#define HOTEL_QUESTION              400     //"Czy mogę wziąć hotel w M?"
#define HOTEL_ANSWER                500     //"Możesz wziąć hotel w M"

#define HALL_QUESTION               600     //"Czy mogę wziąć S w M?"
#define HALL_ANSWER                 700     //"Możesz wziąć S w M"

#define SIGN_IN_END                 800     //"Zapisy na konkurs w M się skończyły"
#define HOTEL_BOOKED                900     //"Mam hotel w M"
#define COMPETITION_END             1000    //"Koniec konkursu"

// HERE'S THE DEFINITION OF MUTEXES
pthread_mutex_t mutex1[16];

int myrandom (int i) { return std::rand()%i;}

Process::Process() {
    MPI_Comm_rank(MPI_COMM_WORLD, &str.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &str.size);
    printf("rank: %d, size: %d\n", str.rank, str.size);
    str.city = -1;
    str.hall = -1;
    str.state = PRESTATE;
    str.hotelSlots = 30;
    str.cityOfCompetitionWeTakePartIn = -1;
    str.idOfCompetitionWeTakePartIn = -1;
    str.clock = 0;
    str.competitionClock = 0;
    srand((unsigned int) time(NULL) + str.rank * 20);
}

Process::Process(long i, long i1, long i2) {
    Process();
    str.numberOfCities = i;
    str.numberOfHalls = i1;
    str.numberOfRoomsInHotel = i2;
}

/*
void incrementAfterMPISend(structToSend ourStr) {
    ourStr.clock[ourStr.rank]++;
}
*/

// returns -1 when second argument clock is greater than first, 0 when they are equal, 1 when first is greater
// and 100 when they are incomparable
//int compareClocks(vector<int> ourClock, vector<int> receivedClock) {
//    int output = 0;
//    for (int i = 0; i < ourClock.size(); i++) {
//        if (ourClock[i] < receivedClock[i]) {
//            if (!output)
//                output = -1;
//            else if (output == 1) {
//                output = 100;
//                break;
//            } else
//                continue;
//        } else if (ourClock[i] == receivedClock[i]) {
//            continue;
//        } else {
//            if (!output)
//                output = 1;
//            else if (output == 1) {
//                output = 100;
//                break;
//            }
//        }
//    }
//    return output;
//}

/*
void incrementAfterMPIRecv(structToSend ourStr, vector<int> receivedClock) {
    incrementAfterMPISend(ourStr);
    for (int i = 0; i < receivedClock.size(); i++) {
        ourStr.clock[i] = ourStr.clock[i] >= receivedClock[i] ? ourStr.clock[i] : receivedClock[i];
    }
}
*/

void Process::sendMessagesAskingIfCompetitionIsHeld(structToSend str) {
    int buf[2] = {0, 1};
    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
    str.clock++;
    buf[0] = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            printf("Sending messages to %d process to check whether any competitions is held. I am %d\n", i, str.rank);
            MPI_Send(buf, 2, MPI_INT, i, COMPETITION_QUESTION, MPI_COMM_WORLD);
        }
    }
    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
}

void Process::sendMessagesAskingHotel(structToSend str) {
    pthread_mutex_lock(&mutex1[CITY_MUTEX]);
    int buf[2] = {0, (int) str.city};
    pthread_mutex_unlock(&mutex1[CITY_MUTEX]);
    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
    str.clock++;
    buf[0] = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            printf("Sending messages to %d process to get hotel. I am %d\n", i, str.rank);
            MPI_Send(buf, 2, MPI_INT, i, HOTEL_QUESTION, MPI_COMM_WORLD);
        }
    }
    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
}

void Process::sendMessagesAskingHall(structToSend str) {
    pthread_mutex_lock(&mutex1[CITY_MUTEX]);
    pthread_mutex_lock(&mutex1[HALL_MUTEX]);
    int buf[3] = {0, (int)str.city, (int)str.hall};
    pthread_mutex_unlock(&mutex1[HALL_MUTEX]);
    pthread_mutex_unlock(&mutex1[CITY_MUTEX]);
    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
    str.clock++;
    buf[0] = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            printf("Sending messages to %d process to get hall. I am %d\n", i, str.rank);
            MPI_Send(buf, 3, MPI_INT, i, HALL_QUESTION, MPI_COMM_WORLD);
        }
    }
    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
}

void Process::behaviour() { // sendy

    printf("Sending messages to check whether any competitions is held.\n");
    printf("FROM_STRUCT: size=%d, rank=%d, city=%ld\n", str.size, str.rank, str.city);
    sendMessagesAskingIfCompetitionIsHeld(str);
    // Create a thread, which will receive questions "Do you organize a competition?"
    pthread_t threadA;
    pthread_t threadB;
    pthread_t threadC;
    pthread_t threadD;
    pthread_create(&threadA, NULL, Process::doYouOrganizeResponder, &str);
    pthread_create(&threadB, NULL, Process::someoneOrganisesResponder, &str);
    pthread_create(&threadC, NULL, Process::canIHavePlaceInHotelResponder, &str);
    pthread_create(&threadD, NULL, Process::canITakeTheHallResponder, &str);
    int buf;
    int tabToBeSent[2];
    int recvBooking;
    vector<int> processesToBeInvited;
    MPI_Status status;

    /* START ALGORITHM */
    while (true) {
        if (str.state == 2000000) break;
        //ask if someone organize competition
        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        str.state = ASK_ORGANIZATION;
        Process::sendMessagesAskingIfCompetitionIsHeld(str);
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

        //wait until threadB set state with role
        while (true) {
            pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            if (str.state != ASK_ORGANIZATION) break;
            pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
        }

        if (str.state == DECIDED_TO_PARTICIPATE) {
            /* MAIN SCENARIO */
            //wait for info from organizer of competition about end of sign up
            MPI_Recv(&buf, 1, MPI_INT, str.idOfCompetitionWeTakePartIn, SIGN_IN_END, MPI_COMM_WORLD, &status);

            //ask if you can take hotel and change state
            pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            str.state = ASK_HOTEL;
            //TODO: save clock of send to compare later by responder
            Process::sendMessagesAskingHotel(str);
            pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

            //recv agreement (size - numberOfRooms)
            while (true) {
                MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD, &status);

                //increment counter of replies
                pthread_mutex_lock(&mutex1[STATE_MUTEX]);
                pthread_mutex_lock(&mutex1[HOTEL_AGREED_MUTEX]);
                str.hotelAgreed++;

                //check if you have a lot of agrees - then you have hotel, so left loop
                if (str.hotelAgreed >= str.size - str.numberOfRoomsInHotel) {
                    str.state = WAITING_FOR_END;

                    //send info that you have hotel
                    tabToBeSent[1] = 1;
                    pthread_mutex_lock(&mutex1[ID_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, str.idOfCompetitionWeTakePartIn, HOTEL_BOOKED, MPI_COMM_WORLD);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                    pthread_mutex_unlock(&mutex1[ID_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);

                    //left critical section and left loop to wait for end of competition
                    pthread_mutex_unlock(&mutex1[HOTEL_AGREED_MUTEX]);
                    pthread_mutex_lock(&mutex1[STATE_MUTEX]);
                    break;
                }

                pthread_mutex_unlock(&mutex1[HOTEL_AGREED_MUTEX]);
                pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            }

            //wait for end of competition
            MPI_Recv(&buf, 1, MPI_INT, str.idOfCompetitionWeTakePartIn, COMPETITION_END, MPI_COMM_WORLD, &status);
            pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            str.state = AFTER_COMPETITION_IN_HOTEL;
            pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

            //sit in hotel for some time
            sleep(3);

            //first set to PRESTATE - responder now respond agree to asking processes - no new Process on list to respond
            pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            str.state = PRESTATE;
            pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

            //here left hotel and send agree to process which are on waiting list
            pthread_mutex_lock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HOTEL_MUTEX]);
            tabToBeSent[1] = 1;
            pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
            str.clock++;
            tabToBeSent[0] = str.clock;
            while (!str.listOfProcessesWantingPlaceInOurHotel.empty()) {
                MPI_Send(tabToBeSent, 2, MPI_INT, str.listOfProcessesWantingPlaceInOurHotel.back(), HOTEL_ANSWER,
                         MPI_COMM_WORLD);
                str.listOfProcessesWantingPlaceInOurHotel.pop_back();
            }
            pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
            pthread_mutex_unlock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HOTEL_MUTEX]);

            //clear data which are terminated(all needed?)
            str.idOfCompetitionWeTakePartIn = -1;
            str.city = -1;
            str.cityOfCompetitionWeTakePartIn = -1;
            str.hall = -1;
            str.potentialUsers.clear();
            str.signedUsers.clear();
            str.listOfProcessesWantingPlaceInOurHotel.clear();

        } else {
            /* ORGANIZE COMPETITION */

            //choose city and hall
            str.city = rand() % str.numberOfCities;
            str.hall = rand() % str.numberOfHalls;

            //ask if you can take it and change state to wait for reply
            pthread_mutex_lock(&mutex1[STATE_MUTEX]);
            Process::sendMessagesAskingHall(str);
            str.state = ASK_HALL;
            pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

            //wait for n-1 reply
            while (true) {
                MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, HALL_ANSWER, MPI_COMM_WORLD, &status);

                //increment counter of replies
                pthread_mutex_lock(&mutex1[STATE_MUTEX]);
                pthread_mutex_lock(&mutex1[HALL_AGREED_MUTEX]);
                str.hallAgreed++;

                //check if you have a lot of agrees - then you have hall, so left loop
                if (str.hotelAgreed >= str.size - 1) {
                    str.state = ASK_INVITES;
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    //send invites to other processes (== number of rooms) and add them on potential users list
                    //TODO: randomize
                    processesToBeInvited = randomize(str);
                    pthread_mutex_lock(&mutex1[POTENTIAL_USERS_MUTEX]);
                    pthread_mutex_lock(&mutex1[CITY_MUTEX]);
                    for (int i = 0; i < str.numberOfRoomsInHotel; i++) {
                        tabToBeSent[1] = (int)str.city;
                        MPI_Send(&buf, 1, MPI_INT, processesToBeInvited[i], COMPETITION_ANSWER, MPI_COMM_WORLD);
                        str.potentialUsers.push_back(processesToBeInvited[i]);
                    }
                    pthread_mutex_unlock(&mutex1[CITY_MUTEX]);
                    pthread_mutex_unlock(&mutex1[POTENTIAL_USERS_MUTEX]);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                    //left critical section and left loop to wait for all potential responsed
                    pthread_mutex_unlock(&mutex1[HALL_AGREED_MUTEX]);
                    pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
                    break;
                }
                pthread_mutex_unlock(&mutex1[HALL_AGREED_MUTEX]);
                pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
            }

            //wait for end of sign up to send end
            while(true) {
                MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD, &status);

                //remove from potential and if positive confirm -> push on signed users
                pthread_mutex_lock(&mutex1[STATE_MUTEX]);
                pthread_mutex_lock(&mutex1[POTENTIAL_USERS_MUTEX]);
                pthread_mutex_lock(&mutex1[SIGNED_USERS_MUTEX]);

                //remove from potential
                std::vector<int>::iterator position = std::find(str.potentialUsers.begin(), str.potentialUsers.end(), status.MPI_SOURCE);
                if (position != str.potentialUsers.end()) {// == myVector.end() means the element was not found
                    str.potentialUsers.erase(position);
                }

                //if positive add on signed in
                if(buf == 1) {
                    str.signedUsers.push_back(status.MPI_SOURCE);
                }

                //check if potentialUsers is empty -> then close sign in and send it to participants
                if(str.potentialUsers.empty()) {
                    str.state = RECV_HOTEL_RESERVATIONS;
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    pthread_mutex_lock(&mutex1[COMPETITION_CLOCK_MUTEX]);
                    str.competitionClock = str.clock;
                    str.clock++;
                    int sendFields[3];
                    sendFields[0] = str.clock;
                    sendFields[1] = str.competitionClock;
                    sendFields[2] = buf;
                    //send info about close sign in - DON'T CLEAR signedUsers - you need to know how many hotelRes recv
                    for(std::vector<int>::size_type i = 0; i != str.signedUsers.size(); i++) {
                        MPI_Send(sendFields, 3, MPI_INT, str.signedUsers.back(), SIGN_IN_END, MPI_COMM_WORLD);
                    }
                    pthread_mutex_unlock(&mutex1[COMPETITION_CLOCK_MUTEX]);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                    pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
                    pthread_mutex_unlock(&mutex1[POTENTIAL_USERS_MUTEX]);
                    pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
                    break;
                }
                pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
                pthread_mutex_unlock(&mutex1[POTENTIAL_USERS_MUTEX]);
                pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
            }

            //wait for participants to take hotel
            recvBooking = 0;
            while(true) {
                MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, HOTEL_BOOKED, MPI_COMM_WORLD, &status);

                pthread_mutex_lock(&mutex1[STATE_MUTEX]);
                pthread_mutex_lock(&mutex1[SIGNED_USERS_MUTEX]);
                pthread_mutex_lock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX]);
                //count confirmed participant from signed_users
                recvBooking++;

                //check if you have a lot of booking -> then end competition and left hall in same moment
                if(recvBooking >= str.signedUsers.size()) {
                    //change state to PRESTATE
                    str.state=PRESTATE;
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    pthread_mutex_lock(&mutex1[COMPETITION_CLOCK_MUTEX]);
                    str.competitionClock = -1; // you are not an organizer and you don't have competition priority
                    pthread_mutex_lock(&mutex1[COMPETITION_CLOCK_MUTEX]);
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    tabToBeSent[1] = 1;
                    //send to participants info about end of competition
                    while(!str.signedUsers.empty()) {
                        MPI_Send(tabToBeSent, 2, MPI_INT, str.signedUsers.back(), COMPETITION_END, MPI_COMM_WORLD);
                        str.signedUsers.pop_back();
                    }
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    //send agree to process waiting for your hall
                    while(!str.listOfProcessesWantingPlaceInOurHall.empty()) {
                        MPI_Send(tabToBeSent, 2, MPI_INT, str.listOfProcessesWantingPlaceInOurHall.back(), HALL_ANSWER, MPI_COMM_WORLD);
                        str.listOfProcessesWantingPlaceInOurHall.pop_back();
                    }
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                    pthread_mutex_unlock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX]);
                    pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
                    pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
                    break;
                }
                pthread_mutex_unlock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX]);
                pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
                pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
            }
            //clear data which are terminated(all needed?)
            str.idOfCompetitionWeTakePartIn = -1;
            str.city = -1;
            str.cityOfCompetitionWeTakePartIn = -1;
            str.hall = -1;
            str.potentialUsers.clear();
            str.signedUsers.clear();
            str.listOfProcessesWantingPlaceInOurHall.clear();
        }
    }

pthread_join(threadA,NULL);
pthread_join(threadB,NULL);
}

//COMPETITION_QUESTION RESPONDER
void *Process::doYouOrganizeResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int buf = 1;
    int decision[2];
    MPI_Status status;

    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;

        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, COMPETITION_QUESTION, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        pthread_mutex_lock(&mutex1[POTENTIAL_USERS_MUTEX]);
        pthread_mutex_lock(&mutex1[SIGNED_USERS_MUTEX]);

        // If we have a free slot then answer with city ID
        if (sharedData->state == ASK_INVITES && freeSlotInVectors(sharedData)) {
            decision[1] = (int)sharedData->city;
            sharedData->potentialUsers.push_back(status.MPI_SOURCE);
        }
            // If any condition is not satisfied, send -1 as city ID
        else {
            decision[1] = -1;
        }
        pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
        sharedData->clock++;
        decision[0] = sharedData->clock;
        //send msg
        MPI_Send(&decision, 1, MPI_INT, status.MPI_SOURCE, COMPETITION_ANSWER, MPI_COMM_WORLD);
        pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
        pthread_mutex_unlock(&mutex1[SIGNED_USERS_MUTEX]);
        pthread_mutex_unlock(&mutex1[POTENTIAL_USERS_MUTEX]);
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
    }
    return nullptr;
}

bool Process::freeSlotInVectors(structToSend *str) {
    return (str->potentialUsers.size() + str->signedUsers.size() >= str->hotelSlots);
}


int generateRole() {
    int randomNumber = rand() % 100 + 1;
    printf("%d\n", randomNumber);
    if (randomNumber < 20) {
        return DECIDED_TO_ORGANIZE;
    }
    return DECIDED_TO_PARTICIPATE;
}

//COMPETITION_ANSWER RESPONDER (THIS THREAD SET ROLE AND STATE)
void *Process::someoneOrganisesResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int howManyRespondedThatDoNotOrganise = 0;
    int buf;
    int tabToBeSent[2];
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        // If I waited for 200, so I wait for responses about organising a competition
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, COMPETITION_ANSWER, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        if (sharedData->state == ASK_ORGANIZATION) {
            if (buf != -1) {
                int newState = generateRole();
                if (newState == DECIDED_TO_ORGANIZE) {
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    tabToBeSent[1] = -1;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                } else {
                    //we are participant, so set variables and then send confirm
                    pthread_mutex_lock(&mutex1[CITY_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);
                    pthread_mutex_lock(&mutex1[ID_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);
                    sharedData->idOfCompetitionWeTakePartIn = status.MPI_SOURCE;
                    sharedData->cityOfCompetitionWeTakePartIn = buf;
                    pthread_mutex_unlock(&mutex1[ID_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);
                    pthread_mutex_unlock(&mutex1[CITY_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX]);
                    // Participate in this competition
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    tabToBeSent[1] = 1;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                }
                //We chosen state (org or part), so counter=0 and set state in struct
                howManyRespondedThatDoNotOrganise = 0;
                sharedData->state = newState;
            } else {
                howManyRespondedThatDoNotOrganise++;
            }
            //check if you have a lot of "NO" and you must be organizer - also you have role so counter = 0
            if (howManyRespondedThatDoNotOrganise == sharedData->size - 1) {
                sharedData->state = DECIDED_TO_ORGANIZE;
                howManyRespondedThatDoNotOrganise = 0;
            }
        } else {
            //in other states just send you don't want to participate
            pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
            sharedData->clock++;
            tabToBeSent[0] = sharedData->clock;
            tabToBeSent[1] = -1;
            MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
            pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
        }
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
    }
    return nullptr;
}


//HOTEL_QUESTION RESPONDER
void *Process::canIHavePlaceInHotelResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int buf;
    int tabToBeSent[2];
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, HOTEL_QUESTION, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        // If we are in state that we will ask for the hotel or we are in hotel now
        if (sharedData->state == ASK_HOTEL || sharedData->state == WAITING_FOR_END ||
            sharedData->state == AFTER_COMPETITION_IN_HOTEL) {
            // We agree if someone wants hotel in not our city
            if (buf != sharedData->cityOfCompetitionWeTakePartIn) {
                pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                tabToBeSent[1] = 1;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
                pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                //  but when it's in our city and we will just add them to list of processes, which we will reply
                // after we give back place in hotel (if his request have bigger priority)
            } else {
                //if he have bigger priority, then we send agree
                if (true) { //TODO: condition who have bigger priority
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    tabToBeSent[1] = 1;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                }
                    //if he have less priority, then we push him on list of waiting
                else {
                    pthread_mutex_lock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HOTEL_MUTEX]);
                    sharedData->listOfProcessesWantingPlaceInOurHotel.push_back(status.MPI_SOURCE);
                    pthread_mutex_unlock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HOTEL_MUTEX]);
                }
            }
            // if we are in any other state, we agree to take a place in hotel
        } else {
            pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
            sharedData->clock++;
            tabToBeSent[0] = sharedData->clock;
            tabToBeSent[1] = 1;
            MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
            pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
        }
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);
    }
    return nullptr;
}

//HALL_QUESTION RESPONDER
void *Process::canITakeTheHallResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int buf[2]; // buf[0] is city id, buf[1] is hall id
    int tabToBeSent[2];
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        MPI_Recv(&buf, 2, MPI_INT, MPI_ANY_SOURCE, HALL_QUESTION, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&mutex1[STATE_MUTEX]);
        // If we are in state that we will ask for the hotel or we are in hotel now
        if (sharedData->state == ASK_HALL || sharedData->state == ASK_INVITES ||
            sharedData->state == RECV_HOTEL_RESERVATIONS) {
            if(buf[0] == sharedData->city && buf[1] == sharedData->hall) { // if he wants our hall
                if(true) { // if his priority is higher
                    pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    tabToBeSent[1] = 1;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
                    pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
                } else {
                    pthread_mutex_lock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX]);
                    sharedData->listOfProcessesWantingPlaceInOurHall.push_back(status.MPI_SOURCE);
                    pthread_mutex_unlock(&mutex1[LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX]);
                }
            } else { // if not our hall then agree
                pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                tabToBeSent[1] = 1;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
                pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
            }
        } else { // in any other state just send consent
            pthread_mutex_lock(&mutex1[CLOCK_MUTEX]);
            sharedData->clock++;
            tabToBeSent[0] = sharedData->clock;
            tabToBeSent[1] = 1;
            MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
            pthread_mutex_unlock(&mutex1[CLOCK_MUTEX]);
        }
        pthread_mutex_unlock(&mutex1[STATE_MUTEX]);

    }
    return nullptr;
}

vector<int> Process::randomize(structToSend str) {
    vector<int> newVector;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            newVector.push_back(i);
        }
    }
    random_shuffle(newVector.begin(), newVector.end(), myrandom);
    return newVector;
}
