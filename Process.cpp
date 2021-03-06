//
// Created by sefir on 06.06.17.
//

#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include "Process.h"
#include <algorithm>


using namespace std;

#define PRESTATE                    1000    //Przed zadaniem pytan
#define ASK_ORGANIZATION            1001    //Po zadaniu pytan(100) - czeka na odpowiedzi(200) i po nich decyduje kim jest (wysyla (300))

#define DECIDED_TO_PARTICIPATE      14      //Uczestnik - czeka na (800) przed spytaniem o hotel(400)
#define ASK_HOTEL                   15      //Uczestnik - czekajacy na hotel i po zgodzie((500) * n-H) wysylajacy potwierdzenie do organizatora(900)
#define WAITING_FOR_END             16      //Uczestnik - czekajacy na info o zakonczeniu konkrsu(1000)
#define AFTER_COMPETITION_IN_HOTEL  17      //Uczestnik - po konkursie w czasie gdy jeszcze okupuje hotel - potem rozsyla zgody czekajacym na hotel(500)

#define DECIDED_TO_ORGANIZE         20      //Organizator - przed spytaniem o sale(600) (moze niepotrzebny?)
#define ASK_HALL                    21      //Organizator - czekajacy na sale i po zgodzie((700) * n-1) wysyla H zaproszen(type 200)
#define ASK_INVITES                 22      //Organizator - czeka na odpowiedzi(300) az lista potencjalnych bedzie pusta; wtedy rozsyla(800) PS. TU WaTEK DO ODBIERANIA (100) zmienia strategie
#define RECV_HOTEL_RESERVATIONS     23      //Organizator - czeka na info o rezerwacjach ((900) * C), rozsyla koniec konkursu(1000) i zgode czekajacym na sale(700)

#define COMPETITION_QUESTION        100     //"Czy organizujesz konkurs?"
#define COMPETITION_ANSWER          200     //"Organizuje konkurs w M"
#define COMPETITION_CONFIRM         300     //"Bede"/"Nie bede"

#define HOTEL_QUESTION              400     //"Czy moge wziac hotel w M?"
#define HOTEL_ANSWER                500     //"Mozesz wziac hotel w M"

#define HALL_QUESTION               600     //"Czy moge wziac S w M?"
#define HALL_ANSWER                 700     //"Mozesz wziac S w M"

#define SIGN_IN_END                 800     //"Zapisy na konkurs w M sie skonczyly"
#define HOTEL_BOOKED                900     //"Mam hotel w M"
#define COMPETITION_END             1000    //"Koniec konkursu"

// HERE'S THE DEFINITION OF MUTEXES
pthread_mutex_t strMutex;

int myrandom(int i) { return std::rand() % i; }

Process::Process(long i, long i1, long i2) {
    MPI_Comm_rank(MPI_COMM_WORLD, &str.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &str.size);
    //printf("rank: %d, size: %d\n", str.rank, str.size);
    str.city = -1;
    str.hall = -1;
    str.cityOfCompetitionWeTakePartIn = -1;
    str.idOfCompetitionWeTakePartIn = -1;
    str.clock = 0;
    str.competitionClock = 0;
    srand((unsigned int) time(NULL) + str.rank * 20);
    str.numberOfCities = i;
    str.numberOfHalls = i1;
    str.numberOfRoomsInHotel = i2;
    str.hotelAgreed = 0;
    str.hallAgreed = 0;
    str.hotelRequestClock = -1;
    str.hallRequestClock = -1;
    str.potentialUsers.clear();
    str.signedUsers.clear();
    str.vectorOfHotelRequestsToRespond.clear();
    str.listOfProcessesWantingPlaceInOurHotel.clear();
    str.listOfProcessesWantingPlaceInOurHall.clear();
    str.lastHotelRequestFromProcessesList.assign((unsigned long) str.size, 0);
    pthread_mutex_lock(&strMutex);
    printInfo("Zmieniam stan na PRESTATE");
    str.clock++;
    str.state = PRESTATE;
    pthread_mutex_unlock(&strMutex);
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
string Process::getState(int state) {
    if (state == PRESTATE) return "PRESTATE";
    if (state == ASK_ORGANIZATION) return "ASK_ORGANIZATION";
    if (state == DECIDED_TO_PARTICIPATE) return "DECIDED_TO_PARTICIPATE";
    if (state == ASK_HOTEL) return "ASK_HOTEL";
    if (state == WAITING_FOR_END) return "WAITING_FOR_END";
    if (state == AFTER_COMPETITION_IN_HOTEL) return "AFTER_COMPETITION_IN_HOTEL";
    if (state == DECIDED_TO_ORGANIZE) return "DECIDED_TO_ORGANIZE";
    if (state == ASK_HALL) return "ASK_HALL";
    if (state == ASK_INVITES) return "ASK_INVITES";
    if (state == RECV_HOTEL_RESERVATIONS) return "RECV_HOTEL_RESERVATIONS";
    return "NO_DATA";
}

string Process::getStateFromThread(structToSend *str) {
    if (str->state == PRESTATE) return "PRESTATE";
    if (str->state == ASK_ORGANIZATION) return "ASK_ORGANIZATION";
    if (str->state == DECIDED_TO_PARTICIPATE) return "DECIDED_TO_PARTICIPATE";
    if (str->state == ASK_HOTEL) return "ASK_HOTEL";
    if (str->state == WAITING_FOR_END) return "WAITING_FOR_END";
    if (str->state == AFTER_COMPETITION_IN_HOTEL) return "AFTER_COMPETITION_IN_HOTEL";
    if (str->state == DECIDED_TO_ORGANIZE) return "DECIDED_TO_ORGANIZE";
    if (str->state == ASK_HALL) return "ASK_HALL";
    if (str->state == ASK_INVITES) return "ASK_INVITES";
    if (str->state == RECV_HOTEL_RESERVATIONS) return "RECV_HOTEL_RESERVATIONS";
    return "NO_DATA";
}


void Process::printInfo(string info) {
    cout << str.clock << " , PID: " << str.rank << " , STATE: " << Process::getState(str.state) << " , INFO: " << info << endl;
}

void Process::printInfoFromThread(string info, structToSend *str) {
    cout << str->clock << " , PID: " << str->rank << " , STATE: " << Process::getStateFromThread(str) << " , INFO: " << info << endl;
}

void Process::sendMessagesAskingIfCompetitionIsHeld() {
    int buf[2] = {0, 1};
    str.clock++;
    buf[0] = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            MPI_Send(buf, 2, MPI_INT, i, COMPETITION_QUESTION, MPI_COMM_WORLD);
        }
    }
    printInfo("Wyslalem pytanie czy ktos organizuje konkurs?");
}

void Process::sendMessagesAskingHotel() {
    int buf[4] = {0, str.cityOfCompetitionWeTakePartIn, str.competitionClock, str.idOfCompetitionWeTakePartIn};
    str.clock++;
    buf[0] = str.clock;
    str.hotelRequestClock = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            MPI_Send(buf, 4, MPI_INT, i, HOTEL_QUESTION, MPI_COMM_WORLD);
        }
    }
    printInfo("Wyslalem pytanie czy moge wziac miejsce w hotelu w miescie " + to_string(buf[1]) + "?");
}

void Process::sendMessagesAskingHall() {
    int buf[3] = {0, (int) str.city, (int) str.hall};
    str.clock++;
    buf[0] = str.clock;
    str.hallRequestClock = str.clock;
    for (int i = 0; i < str.size; i++) {
        if (i != str.rank) {
            MPI_Send(buf, 3, MPI_INT, i, HALL_QUESTION, MPI_COMM_WORLD);
        }
    }
    printInfo("Wyslalem pytanie czy moge wziac sale " + to_string(buf[2]) + " w miescie " + to_string(buf[1]) + "?");
}

void Process::behaviour() { // sendy
    /*
    printf("Sending messages to check whether any competitions is held.\n");
    printf("FROM_STRUCT: size=%d, rank=%d, city=%ld\n", str.size, str.rank, str.city);
    printf("FROM_STRUCT: nrCity=%ld, nrHalls=%ld, nrHotels=%ld\n", str.numberOfCities, str.numberOfHalls,
           str.numberOfRoomsInHotel);
    */
    // Create a thread, which will receive questions "Do you organize a competition?"
    pthread_t threadA;
    pthread_t threadB;
    pthread_t threadC;
    pthread_t threadD;
    pthread_create(&threadA, NULL, Process::doYouOrganizeResponder, &str);
    pthread_create(&threadB, NULL, Process::someoneOrganisesResponder, &str);
    pthread_create(&threadC, NULL, Process::canIHavePlaceInHotelResponder, &str);
    pthread_create(&threadD, NULL, Process::canITakeTheHallResponder, &str);
    //int buf;
    int tabToBeSent[2];
    int recvBooking;
    int recv2Tab[2];
    int recv3Tab[3];
    vector<int> processesToBeInvited;
    MPI_Status status;

    /* START ALGORITHM */
    while (true) {
        if (str.state == 2000000) break;
        //ask if someone organize competition
        pthread_mutex_lock(&strMutex);
        printInfo("Zmieniam stan na ASK_ORGANIZATION");
        str.clock++;
        str.state = ASK_ORGANIZATION;
        Process::sendMessagesAskingIfCompetitionIsHeld();
        pthread_mutex_unlock(&strMutex);

        //wait until threadB set state with role
        while (true) {
            pthread_mutex_lock(&strMutex);
            if (str.state != ASK_ORGANIZATION) {
                pthread_mutex_unlock(&strMutex);
                break;
            }
            pthread_mutex_unlock(&strMutex);
        }

        if (str.state == DECIDED_TO_PARTICIPATE) {
            /* MAIN SCENARIO */
            //wait for info from organizer of competition about end of sign up
            MPI_Recv(recv3Tab, 3, MPI_INT, str.idOfCompetitionWeTakePartIn, SIGN_IN_END, MPI_COMM_WORLD, &status);

            //ask if you can take hotel and change state
            pthread_mutex_lock(&strMutex);
            str.clock = max(str.clock, recv3Tab[0]) + 1;
            printInfo("Odebralem informacje od organizatora(" + to_string(status.MPI_SOURCE) +
                      ") o zakonczeniu zapisow na konkurs");
            str.competitionClock = recv3Tab[1];

            printInfo("Zmieniam stan na ASK_HOTEL");
            str.clock++;
            str.state = ASK_HOTEL;
            Process::sendMessagesAskingHotel();

            //TODO: send info to all processes asked you for this hotel
            bool priority = false;
            int tabToSend[2];
            while(!str.vectorOfHotelRequestsToRespond.empty()) {

                if (str.hotelRequestClock == -1) priority = false;
                else if (str.vectorOfHotelRequestsToRespond.back().competitionClockOfRequest < str.competitionClock) priority = false;
                else if (str.vectorOfHotelRequestsToRespond.back().competitionClockOfRequest == str.competitionClock) {
                    if (str.vectorOfHotelRequestsToRespond.back().competitionIdOfRequest < str.idOfCompetitionWeTakePartIn) priority = false;
                    else if (str.vectorOfHotelRequestsToRespond.back().competitionIdOfRequest == str.idOfCompetitionWeTakePartIn) {
                        if (str.vectorOfHotelRequestsToRespond.back().clockOfRequest < str.hotelRequestClock) priority = false;
                        else if (str.vectorOfHotelRequestsToRespond.back().clockOfRequest == str.hotelRequestClock) {
                            if (str.vectorOfHotelRequestsToRespond.back().processOfRequest < str.rank) priority = false;
                            else priority = true;
                        } else if (str.vectorOfHotelRequestsToRespond.back().clockOfRequest > str.hotelRequestClock) priority = true;
                    } else if (str.vectorOfHotelRequestsToRespond.back().competitionIdOfRequest > str.idOfCompetitionWeTakePartIn) priority = true;
                } else if (str.vectorOfHotelRequestsToRespond.back().competitionClockOfRequest > str.competitionClock) priority = true;

                if(priority){ //I HAVE PRIORITY
                    str.clock++;
                    str.lastHotelRequestFromProcessesList[str.vectorOfHotelRequestsToRespond.back().processOfRequest] = str.vectorOfHotelRequestsToRespond.back().clockOfRequest; //set val to remember which caption fill this field in feature

                    //add if he is not on vector
                    std::vector<int>::iterator positionInVector = std::find(str.listOfProcessesWantingPlaceInOurHotel.begin(),str.listOfProcessesWantingPlaceInOurHotel.end(),
                                                                            str.vectorOfHotelRequestsToRespond.back().processOfRequest);
                    if (positionInVector == str.listOfProcessesWantingPlaceInOurHotel.end()) {// == myVector.end() means the element was not found
                        str.listOfProcessesWantingPlaceInOurHotel.push_back(str.vectorOfHotelRequestsToRespond.back().processOfRequest); //push_back only if he is not on list
                    }
                    printInfo("Odlozylem proces " + to_string(str.vectorOfHotelRequestsToRespond.back().processOfRequest) +
                                        " na kolejke oczekujacych na hotel w miescie " + to_string(str.vectorOfHotelRequestsToRespond.back().cityOfRequest));
                }
                else { //HE HAVE PRIORITY
                    tabToSend[1] = str.vectorOfHotelRequestsToRespond.back().clockOfRequest; //respond with time of request to make sure this process, that this response came for his newest request
                    str.clock++;
                    tabToSend[0] = str.clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, str.vectorOfHotelRequestsToRespond.back().processOfRequest, HOTEL_ANSWER, MPI_COMM_WORLD);
                    printInfo("Wyslalem procesowi " + to_string(str.vectorOfHotelRequestsToRespond.back().processOfRequest) +
                                        " informacje, ze moze wziac hotel w miescie " + to_string(str.vectorOfHotelRequestsToRespond.back().cityOfRequest));
                }
                str.vectorOfHotelRequestsToRespond.pop_back();
            }

            pthread_mutex_unlock(&strMutex);

            //recv agreement (size - numberOfRooms)
            while (true) {
                MPI_Recv(recv2Tab, 2, MPI_INT, MPI_ANY_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD, &status);

                pthread_mutex_lock(&strMutex);
                str.clock = max(str.clock, recv2Tab[0]) + 1;
                printInfo("Odebralem zgode na zajecie hotelu od procesu " + to_string(status.MPI_SOURCE) +
                          " - jest to odpowiedz na moj request z " + to_string(recv2Tab[1]));
                if (str.hotelRequestClock == recv2Tab[1])
                    str.hotelAgreed++; //increment only if you recived agree from now - expected to recv also msg from past

                //check if you have a lot of agrees - then you have hotel, so left loop
                if (str.hotelAgreed >= str.size - str.numberOfRoomsInHotel) {
                    printInfo("ZAJMUJE HOTEL W MIESCIE " + to_string(str.cityOfCompetitionWeTakePartIn));
                    printInfo("Zmieniam stan na WAITING_FOR_END");
                    str.clock++;
                    str.state = WAITING_FOR_END;
                    printInfo("Mam wystarczajaco duzo zgod by zajac hotel w miescie " +
                              str.cityOfCompetitionWeTakePartIn);
                    //send info that you have hotel
                    tabToBeSent[1] = 1;
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, str.idOfCompetitionWeTakePartIn, HOTEL_BOOKED, MPI_COMM_WORLD);
                    printInfo("Wyslalem informacje do organizatora(" + to_string(str.idOfCompetitionWeTakePartIn) +
                              "), ze mam juz hotel");
                    //left critical section and left loop to wait for end of competition
                    pthread_mutex_unlock(&strMutex);
                    break;
                }
                pthread_mutex_unlock(&strMutex);
            }

            //wait for end of competition
            MPI_Recv(recv2Tab, 2, MPI_INT, str.idOfCompetitionWeTakePartIn, COMPETITION_END, MPI_COMM_WORLD, &status);

            pthread_mutex_lock(&strMutex);
            str.clock = max(str.clock, recv2Tab[0]) + 1;
            printInfo("Odebralem informacje od organizatora(" + to_string(status.MPI_SOURCE) +
                      ") o zakonczeniu konkursu");
            printInfo("Zmieniam stan na AFTER_COMPETITION_IN_HOTEL");
            str.clock++;
            str.state = AFTER_COMPETITION_IN_HOTEL;
            pthread_mutex_unlock(&strMutex);

            //sit in hotel for some time
            usleep((__useconds_t) (100000 * ((rand() % 50) + 1))); //rand wait from 0,1 to 5 sek //TODO: maybe some clock_counter?

            pthread_mutex_lock(&strMutex);
            str.clock++;
            printInfo("ZWALNIAM HOTEL W MIESCIE " + to_string(str.cityOfCompetitionWeTakePartIn));
            //here left hotel and send agree to process which are on waiting list
            tabToBeSent[1] = 1;
            str.clock++;
            tabToBeSent[0] = str.clock;
            while (!str.listOfProcessesWantingPlaceInOurHotel.empty()) {
                // set value to send as last request, to make sure this process that he recv right confirm
                tabToBeSent[1] = str.lastHotelRequestFromProcessesList[str.listOfProcessesWantingPlaceInOurHotel.back()];
                MPI_Send(tabToBeSent, 2, MPI_INT, str.listOfProcessesWantingPlaceInOurHotel.back(), HOTEL_ANSWER,
                         MPI_COMM_WORLD);
                str.listOfProcessesWantingPlaceInOurHotel.pop_back();
            }
            printInfo("Wyslalem informacje o zwolnieniu hotelu w miescie " +
                      to_string(str.cityOfCompetitionWeTakePartIn) + " do czekajacych na nia");
            //clear data which are terminated(all needed?)
            str.city = -1;
            str.hall = -1;
            printInfo("Zmieniam stan na PRESTATE");
            str.clock++;
            str.state = PRESTATE;
            str.competitionClock = -1;
            str.potentialUsers.clear();
            str.signedUsers.clear();
            str.cityOfCompetitionWeTakePartIn = -1;
            str.idOfCompetitionWeTakePartIn = -1;
            str.listOfProcessesWantingPlaceInOurHotel.clear();
            str.listOfProcessesWantingPlaceInOurHall.clear();
            str.vectorOfHotelRequestsToRespond.clear();
            str.hotelAgreed = 0;
            str.hallAgreed = 0;
            str.hotelRequestClock = -1;
            str.hallRequestClock = -1;

            pthread_mutex_unlock(&strMutex);

        } else {
            printInfo("Postanowilem zorganizowac konkurs");
            /* ORGANIZE COMPETITION */
            pthread_mutex_lock(&strMutex);
            //choose city and hall
            str.city = rand() % str.numberOfCities;
            str.hall = rand() % str.numberOfHalls;

            //ask if you can take it and change state to wait for reply
            printInfo("Zmieniam stan na ASK_HALL");
            str.clock++;
            str.state = ASK_HALL;
            Process::sendMessagesAskingHall();
            pthread_mutex_unlock(&strMutex);

            //wait for n-1 reply
            while (true) {
                MPI_Recv(recv2Tab, 2, MPI_INT, MPI_ANY_SOURCE, HALL_ANSWER, MPI_COMM_WORLD, &status);

                pthread_mutex_lock(&strMutex);
                str.clock = max(str.clock, recv2Tab[0]) + 1;
                printInfo("Odebralem zgode na zajecie sali od " + to_string(status.MPI_SOURCE));
                str.hallAgreed++;
                printInfo("Mam " + to_string(str.hallAgreed) + " zgod na dostep do sali");

                //check if you have a lot of agrees - then you have hall, so left loop
                if (str.hallAgreed >= str.size - 1) {
                    printInfo("ZAJMUJE SALE "+ to_string(str.hall) + " W MIESCIE " + to_string(str.city));
                    printInfo("Zmieniam stan na ASK_INVITES");
                    str.clock++;
                    str.state = ASK_INVITES;
                    //send invites to other processes (== number of rooms) and add them on potential users list
                    processesToBeInvited = randomize(str);
                    str.clock++;
                    tabToBeSent[0] = str.clock;
                    for (int i = 0; i < str.numberOfRoomsInHotel; i++) {
                        tabToBeSent[1] = (int) str.city;
                        MPI_Send(tabToBeSent, 2, MPI_INT, processesToBeInvited[i], COMPETITION_ANSWER, MPI_COMM_WORLD);
                        str.potentialUsers.push_back(processesToBeInvited[i]);
                    }
                    printInfo("Wyslalem zaproszenie na konkurs do kilku procesow");
                    //left critical section and left loop to wait for all potential responsed
                    pthread_mutex_unlock(&strMutex);
                    break;
                }
                pthread_mutex_unlock(&strMutex);
            }

            //wait for end of sign up to send end
            while (true) {
                MPI_Recv(recv2Tab, 2, MPI_INT, MPI_ANY_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD, &status);

                //remove from potential and if positive confirm -> push on signed users
                pthread_mutex_lock(&strMutex);
                str.clock = max(str.clock, recv2Tab[0]) + 1;
                if (recv2Tab[1] == 1)
                    printInfo("Odebralem potwierdzenie uczestnicwa od " + to_string(status.MPI_SOURCE));
                else
                    printInfo("Odebralem odmowe uczestnicwa od " + to_string(status.MPI_SOURCE));
                //remove from potential
                std::vector<int>::iterator position = std::find(str.potentialUsers.begin(), str.potentialUsers.end(),
                                                                status.MPI_SOURCE);
                if (position != str.potentialUsers.end()) {// == myVector.end() means the element was not found
                    str.potentialUsers.erase(position);
                }

                //if positive add on signed in
                if (recv2Tab[1] == 1) {
                    str.signedUsers.push_back(status.MPI_SOURCE);
                }

                //check if potentialUsers is empty -> then close sign in and send it to participants
                if (str.potentialUsers.empty()) {
                    printInfo("Mam " + to_string(str.signedUsers.size()) + " uczestnikow");
                    printInfo("Zmieniam stan na RECV_HOTEL_RESERVATIONS");
                    str.clock++;
                    str.state = RECV_HOTEL_RESERVATIONS;
                    str.competitionClock = str.clock;

                    int sendFields[3];

                    sendFields[1] = str.competitionClock;
                    sendFields[2] = 1;
                    str.clock++;
                    sendFields[0] = str.clock;
                    //send info about close sign in - DON'T CLEAR signedUsers - you need to know how many hotelRes recv
                    for (int i = 0; i != str.signedUsers.size(); i++) {
                        MPI_Send(sendFields, 3, MPI_INT, str.signedUsers[i], SIGN_IN_END, MPI_COMM_WORLD);
                    }
                    printInfo("Wyslalem informacje o zakonczeniu zapisow do uczestnikow; czekam az zarezerwuja hotel");
                    pthread_mutex_unlock(&strMutex);
                    break;
                }
                pthread_mutex_unlock(&strMutex);
            }


            //check if you have any participants - if not, then end without waiting to recv
            pthread_mutex_lock(&strMutex);
            if (str.signedUsers.size() == 0) {
                printInfo("Nie mam uczestnikow, wiec nie czekam na zgody, koncze od razu konkurs");
                str.competitionClock = -1; // you are not an organizer and you don't have competition priority
                printInfo("ZWALNIAM SALE "+ to_string(str.hall) + " W MIESCIE " + to_string(str.city));
                tabToBeSent[1] = 1;
                str.clock++;
                tabToBeSent[0] = str.clock;
                //send agree to process waiting for your hall
                while (!str.listOfProcessesWantingPlaceInOurHall.empty()) {
                    MPI_Send(tabToBeSent, 2, MPI_INT, str.listOfProcessesWantingPlaceInOurHall.back(), HALL_ANSWER,
                             MPI_COMM_WORLD);
                    str.listOfProcessesWantingPlaceInOurHall.pop_back();
                }
                printInfo("Wyslalem informacje o zwolnieniu sali do czekajacych na nia");
                //clear data which are terminated(all needed?)
                str.city = -1;
                str.hall = -1;
                printInfo("Zmieniam stan na PRESTATE");
                str.clock++;
                str.state = PRESTATE;
                str.competitionClock = -1;
                str.potentialUsers.clear();
                str.signedUsers.clear();
                str.cityOfCompetitionWeTakePartIn = -1;
                str.idOfCompetitionWeTakePartIn = -1;
                str.listOfProcessesWantingPlaceInOurHotel.clear();
                str.listOfProcessesWantingPlaceInOurHall.clear();
                str.hotelAgreed = 0;
                str.hallAgreed = 0;
                str.hotelRequestClock = -1;
                str.hallRequestClock = -1;
                pthread_mutex_unlock(&strMutex);
            } else {
                //unlock mutex which you locked to check state
                pthread_mutex_unlock(&strMutex);
                //wait for participants to take hotel
                recvBooking = 0;
                while (true) {

                    MPI_Recv(recv2Tab, 2, MPI_INT, MPI_ANY_SOURCE, HOTEL_BOOKED, MPI_COMM_WORLD, &status);

                    pthread_mutex_lock(&strMutex);
                    str.clock = max(str.clock, recv2Tab[0]) + 1;
                    printInfo("Odebralem potwierdzenie rezerwacji hotelu od " + to_string(status.MPI_SOURCE));
                    //count confirmed participant from signed_users
                    recvBooking++;

                    //check if you have a lot of booking -> then end competition and left hall in same moment
                    if (recvBooking >= str.signedUsers.size()) {
                        //change state to PRESTATE and prepare to begin algorithm
                        str.competitionClock = -1; // you are not an organizer and you don't have competition priority
                        tabToBeSent[1] = 1;
                        str.clock++;
                        tabToBeSent[0] = str.clock;
                        //send to participants info about end of competition
                        while (!str.signedUsers.empty()) {
                            MPI_Send(tabToBeSent, 2, MPI_INT, str.signedUsers.back(), COMPETITION_END, MPI_COMM_WORLD);
                            str.signedUsers.pop_back();
                        }
                        printInfo("Wyslalem informacje o zakonczeniu konkursu do uczestnikow");
                        printInfo("ZWALNIAM SALE "+ to_string(str.hall) + " W MIESCIE " + to_string(str.city));
                        str.clock++;
                        tabToBeSent[0] = str.clock;
                        //send agree to process waiting for your hall
                        while (!str.listOfProcessesWantingPlaceInOurHall.empty()) {
                            MPI_Send(tabToBeSent, 2, MPI_INT, str.listOfProcessesWantingPlaceInOurHall.back(),
                                     HALL_ANSWER,
                                     MPI_COMM_WORLD);
                            str.listOfProcessesWantingPlaceInOurHall.pop_back();
                        }
                        printInfo("Wyslalem informacje o zwolnieniu sali do czekajacych na nia");
                        //clear data which are terminated(all needed?)
                        str.city = -1;
                        str.hall = -1;
                        printInfo("Zmieniam stan na PRESTATE");
                        str.clock++;
                        str.state = PRESTATE;
                        str.competitionClock = -1;
                        str.potentialUsers.clear();
                        str.signedUsers.clear();
                        str.cityOfCompetitionWeTakePartIn = -1;
                        str.idOfCompetitionWeTakePartIn = -1;
                        str.listOfProcessesWantingPlaceInOurHotel.clear();
                        str.listOfProcessesWantingPlaceInOurHall.clear();
                        str.hotelAgreed = 0;
                        str.hallAgreed = 0;
                        str.hotelRequestClock = -1;
                        str.hallRequestClock = -1;

                        pthread_mutex_unlock(&strMutex);
                        break;
                    }
                    pthread_mutex_unlock(&strMutex);
                }
            }
        }
    }

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
}

//COMPETITION_QUESTION RESPONDER
void *Process::doYouOrganizeResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int decision[2];
    MPI_Status status;

    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;

        MPI_Recv(decision, 2, MPI_INT, MPI_ANY_SOURCE, COMPETITION_QUESTION, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&strMutex);
        sharedData->clock = max(sharedData->clock, decision[0]) + 1;
        printInfoFromThread("Odebralem pytanie o organizacje konkursu od procesu "
                            + to_string(status.MPI_SOURCE), sharedData);
        // If we have a free slot then answer with city ID
        if (sharedData->state == ASK_INVITES && freeSlotInVectors(sharedData)) {
            decision[1] = (int) sharedData->city;
            sharedData->potentialUsers.push_back(status.MPI_SOURCE);
        }
            // If any condition is not satisfied, send -1 as city ID
        else {
            decision[1] = -1;
        }
        sharedData->clock++;
        decision[0] = sharedData->clock;
        //send msg
        MPI_Send(decision, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_ANSWER, MPI_COMM_WORLD);
        if (decision[1] == -1)
            printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                " informacje, ze nie organizuje konkursu", sharedData);
        else
            printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                " informacje, ze organizuje konkurs w miescie " + to_string(decision[1]), sharedData);
        pthread_mutex_unlock(&strMutex);
    }
    return nullptr;
}

bool Process::freeSlotInVectors(structToSend *str) {
    return (str->potentialUsers.size() + str->signedUsers.size() < str->numberOfRoomsInHotel);
}


int generateRole() {
    int randomNumber = rand() % 100 + 1;
    //printf("%d\n", randomNumber);
    if (randomNumber < 20) {
        return DECIDED_TO_ORGANIZE;
    }
    return DECIDED_TO_PARTICIPATE;
}

//COMPETITION_ANSWER RESPONDER (THIS THREAD SET ROLE AND STATE)
void *Process::someoneOrganisesResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int howManyRespondedThatDoNotOrganise = 0;
    int tabToBeSent[2];
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        // If I waited for 200, so I wait for responses about organising a competition
        MPI_Recv(tabToBeSent, 2, MPI_INT, MPI_ANY_SOURCE, COMPETITION_ANSWER, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&strMutex);
        sharedData->clock = max(sharedData->clock, tabToBeSent[0]) + 1;
        if (sharedData->state == ASK_ORGANIZATION) {
            if (tabToBeSent[1] != -1) {
                printInfoFromThread("Odebralem zaproszenie na konkurs od procesu " + to_string(status.MPI_SOURCE), sharedData);
                int newState = generateRole();
                if (newState == DECIDED_TO_ORGANIZE) {
                    printInfoFromThread("Zdecydowalem, ze bede organizatorem konkursu", sharedData);
                    tabToBeSent[1] = -1;
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
                    printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                        " informacje, nie wezme udzialu w konkursie", sharedData);
                    printInfoFromThread("Zmieniam stan na DECIDED_TO_ORGANIZE", sharedData);
                } else {
                    printInfoFromThread("Zdecydowalem, ze bede uczestnikiem konkursu", sharedData);
                    //we are participant, so set variables and then send confirm
                    sharedData->idOfCompetitionWeTakePartIn = status.MPI_SOURCE;
                    sharedData->cityOfCompetitionWeTakePartIn = tabToBeSent[1];
                    // Participate in this competition
                    tabToBeSent[1] = 1;
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
                    printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                        " informacje, ze wezme udzial w konkursie", sharedData);
                    printInfoFromThread("Zmieniam stan na DECIDED_TO_PARTICIPATE", sharedData);
                }
                //We chosen state (org or part), so counter=0 and set state in struct
                howManyRespondedThatDoNotOrganise = 0;
                sharedData->clock++;
                sharedData->state = newState;
            } else {
                howManyRespondedThatDoNotOrganise++;
            }
            //check if you have a lot of "NO" and you must be organizer - also you have role so counter = 0
            if (howManyRespondedThatDoNotOrganise == sharedData->size - 1) {
                printInfoFromThread("Brak konkursow; Musze zostac organizatorem", sharedData);
                printInfoFromThread("Zmieniam stan na DECIDED_TO_ORGANIZE", sharedData);
                sharedData->clock++;
                sharedData->state = DECIDED_TO_ORGANIZE;
                //printf("STAN: %d\n", sharedData->state);
                howManyRespondedThatDoNotOrganise = 0;
            }
        } else {
            //in other states just send you don't want to participate (only if he send that he organize)
            if (tabToBeSent[1] != -1) {
                printInfoFromThread(
                        "Odebralem informacje, ze proces " + to_string(status.MPI_SOURCE) + " organizuje konkurs",
                        sharedData);
                tabToBeSent[1] = -1;
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, COMPETITION_CONFIRM, MPI_COMM_WORLD);
                printInfoFromThread(
                        "Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                        " informacje, nie wezme udzialu w konkursie",
                        sharedData);
            }
            else {
                printInfoFromThread(
                        "Odebralem informacje, ze proces " + to_string(status.MPI_SOURCE) + " nie orgazniuje konkursu",
                        sharedData);
            }
        }
        pthread_mutex_unlock(&strMutex);
    }
    return nullptr;
}


//HOTEL_QUESTION RESPONDER
void *Process::canIHavePlaceInHotelResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int tabToBeSent[2];
    int recvTab[4];
    bool priority = false; //if true, then i have priority; initialized to CLion stop display warning
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        MPI_Recv(recvTab, 4, MPI_INT, MPI_ANY_SOURCE, HOTEL_QUESTION, MPI_COMM_WORLD, &status);
        //recv[0] = clock, recv[1] = city, recv[2] = competitionClock, recv[3] = competitionId
        pthread_mutex_lock(&strMutex);
        sharedData->clock = max(sharedData->clock, recvTab[0]) + 1;
        printInfoFromThread("Odebralem pytanie o hotel w miescie " + to_string(recvTab[1]) + " od procesu "
                            + to_string(status.MPI_SOURCE), sharedData);

        // If we are in state that we asked for the hotel or we are in hotel now
        if (sharedData->state == ASK_HOTEL || sharedData->state == WAITING_FOR_END ||
            sharedData->state == AFTER_COMPETITION_IN_HOTEL) {
            // We agree if someone wants hotel in not our city
            if (recvTab[1] != sharedData->cityOfCompetitionWeTakePartIn) {
                tabToBeSent[1] = recvTab[0]; //respond with time of request to make sure this process, that this response came for his newest request
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
                printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                    " informacje, ze moze wziac hotel w miescie " + to_string(recvTab[1]), sharedData);
            }
                //  but when it's in our city and we will just add them to list of processes, which we will reply
                // after we give back place in hotel (if his request have less priority than our)
            else {
                //check priority : a) less_competitionClock[2] b)less_competitionId[3] c) less_requestClock[0] d) less_rank[status]
                if (sharedData->hotelRequestClock == -1) priority = false;
                else if (recvTab[2] < sharedData->competitionClock) priority = false;
                else if (recvTab[2] == sharedData->competitionClock) {
                    if (recvTab[3] < sharedData->idOfCompetitionWeTakePartIn) priority = false;
                    else if (recvTab[3] == sharedData->idOfCompetitionWeTakePartIn) {
                        if (recvTab[0] < sharedData->hotelRequestClock) priority = false;
                        else if (recvTab[0] == sharedData->hotelRequestClock) {
                            if (sharedData->rank > status.MPI_SOURCE) priority = false;
                            else priority = true;
                        } else if (recvTab[0] > sharedData->hotelRequestClock) priority = true;
                    } else if (recvTab[3] > sharedData->idOfCompetitionWeTakePartIn) priority = true;
                } else if (recvTab[2] > sharedData->competitionClock) priority = true;


                //if he have bigger priority, then we send agree
                if (!priority) {
                    tabToBeSent[1] = recvTab[0]; //respond with time of request to make sure this process, that this response came for his newest request
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
                    printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                        " informacje, ze moze wziac hotel w miescie " + to_string(recvTab[1]),
                                        sharedData);
                }
                    //if he have less priority, then we push him on list of waiting
                else {
                    sharedData->clock++;
                    sharedData->lastHotelRequestFromProcessesList[status.MPI_SOURCE] = recvTab[0]; //set val to remember which caption fill this field in feature

                    //add if he is not on vector
                    std::vector<int>::iterator positionInVector = std::find(sharedData->listOfProcessesWantingPlaceInOurHotel.begin(),sharedData->listOfProcessesWantingPlaceInOurHotel.end(),
                                                                    status.MPI_SOURCE);
                    if (positionInVector == sharedData->listOfProcessesWantingPlaceInOurHotel.end()) {// == myVector.end() means the element was not found
                        sharedData->listOfProcessesWantingPlaceInOurHotel.push_back(status.MPI_SOURCE); //push_back only if he is not on list
                    }
                    printInfoFromThread("Odlozylem proces " + to_string(status.MPI_SOURCE) +
                                        " na kolejke oczekujacych na hotel w miescie " + to_string(recvTab[1]),
                                        sharedData);
                }
            }

        }
        // if we are in DECIDED_TO_PARTICIPATE then we don't know what is our competitionClock, so we don't send yet, put msg to buf and main thread will respond when he change state to ASK_HOTEL
        else if (sharedData->state == DECIDED_TO_PARTICIPATE) {
            //if other hotel - no problem, we dont need it
            if (recvTab[1] != sharedData->cityOfCompetitionWeTakePartIn) {
                tabToBeSent[1] = recvTab[0]; //respond with time of request to make sure this process, that this response came for his newest request
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
                printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                    " informacje, ze moze wziac hotel w miescie " + to_string(recvTab[1]), sharedData);
            }
            else {
                //push back new request to respond in main thread after change state to ASK_HOTEL
                hotelRequestStruct newRequest;
                newRequest.processOfRequest = status.MPI_SOURCE;
                newRequest.cityOfRequest = recvTab[1];
                newRequest.clockOfRequest = recvTab[0];
                newRequest.competitionClockOfRequest = recvTab[2];
                newRequest.competitionIdOfRequest = recvTab[3];
                sharedData->vectorOfHotelRequestsToRespond.push_back(newRequest);
                printInfoFromThread("JESTEM TU KURWA", sharedData);
            }
        }
            // if we are in any other state, we agree to take a place in hotel
        else {
            tabToBeSent[1] = recvTab[0]; //respond with time of request to make sure this process, that this response came for his newest request
            sharedData->clock++;
            tabToBeSent[0] = sharedData->clock;
            MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HOTEL_ANSWER, MPI_COMM_WORLD);
            printInfoFromThread("Wyslalem procesowi " + to_string(status.MPI_SOURCE) +
                                " informacje, ze moze wziac hotel w miescie " + to_string(recvTab[1]), sharedData);
        }
        pthread_mutex_unlock(&strMutex);
    }
    return nullptr;
}

//HALL_QUESTION RESPONDER
void *Process::canITakeTheHallResponder(void *ptr) {
    structToSend *sharedData = (structToSend *) ptr;
    int recvTab[3];
    int tabToBeSent[2];
    bool priority = false; //if true, then i have priority - set to CLion warning stop display
    MPI_Status status;
    while (true) {
        // to disable CLion's verification of endless loop
        if (sharedData->state == 2000000)
            break;
        MPI_Recv(recvTab, 3, MPI_INT, MPI_ANY_SOURCE, HALL_QUESTION, MPI_COMM_WORLD, &status);
        //[0] clock, [1] city, [2] hall

        pthread_mutex_lock(&strMutex);
        sharedData->clock = max(sharedData->clock, recvTab[0]) + 1;
        printInfoFromThread(
                "Odebralem pytanie o sale " + to_string(recvTab[2]) + " w miescie " + to_string(recvTab[1]) +
                " od procesu " + to_string(status.MPI_SOURCE), sharedData);

        // If we are in state that we will ask for the hotel or we are in hotel now
        if (sharedData->state == ASK_HALL || sharedData->state == ASK_INVITES ||
            sharedData->state == RECV_HOTEL_RESERVATIONS) {
            if (recvTab[1] == sharedData->city && recvTab[2] == sharedData->hall) { // if he wants our hall

                //check priority : a) less_clock_of_request b)less_rank
                /*cout << sharedData->clock << " PID: " << sharedData->rank << ", INFO: " << "SPRAWDZAM PRIORITY:" << endl
                     << "RECV_CLOCK: " << recvTab[0] << endl << "MY_REQ_CLOCK: " << sharedData->hallRequestClock << endl
                     << "RECV_RANK: " << status.MPI_SOURCE << endl << "MY_RANK: " << sharedData->rank << endl;*/
                if (sharedData->hallRequestClock == -1)
                    priority = false; //don't want hall (probably impossible, but ..)
                else if (recvTab[0] < sharedData->hallRequestClock) priority = false;
                else if (recvTab[0] == sharedData->hallRequestClock) {
                    if (sharedData->rank > status.MPI_SOURCE) priority = false;
                    else priority = true;
                } else if (recvTab[0] > sharedData->hallRequestClock) priority = true;

                if (!priority) { // if his priority is higher
                    tabToBeSent[1] = 1;
                    sharedData->clock++;
                    tabToBeSent[0] = sharedData->clock;
                    MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
                    printInfoFromThread(
                            "Wyslalem procesowi " + to_string(status.MPI_SOURCE) + " informacje, ze moze wziac sale " +
                            to_string(recvTab[2]) + " w miescie " + to_string(recvTab[1]), sharedData);
                } else { //if our priority is higher
                    sharedData->clock++;
                    sharedData->listOfProcessesWantingPlaceInOurHall.push_back(status.MPI_SOURCE);
                    printInfoFromThread(
                            "Odlozylem proces " + to_string(status.MPI_SOURCE) + " na kolejke oczekujacych na sale " +
                            to_string(recvTab[2]) + " w miescie " + to_string(recvTab[1]), sharedData);
                }
            } else { // if not our hall then agree
                tabToBeSent[1] = 1;
                sharedData->clock++;
                tabToBeSent[0] = sharedData->clock;
                MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
                printInfoFromThread(
                        "Wyslalem procesowi " + to_string(status.MPI_SOURCE) + " informacje, ze moze wziac sale " +
                        to_string(recvTab[2]) + " w miescie " + to_string(recvTab[1]), sharedData);
            }
        } else { // in any other state just send consent
            tabToBeSent[1] = 1;
            sharedData->clock++;
            tabToBeSent[0] = sharedData->clock;
            MPI_Send(tabToBeSent, 2, MPI_INT, status.MPI_SOURCE, HALL_ANSWER, MPI_COMM_WORLD);
            printInfoFromThread(
                    "Wyslalem procesowi " + to_string(status.MPI_SOURCE) + " informacje, ze moze wziac sale " +
                    to_string(recvTab[2]) + " w miescie " + to_string(recvTab[1]), sharedData);
        }
        pthread_mutex_unlock(&strMutex);
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
