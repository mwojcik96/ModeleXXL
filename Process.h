//
// Created by sefir on 06.06.17.
//
#include <string>
#include <mutex>
#include "Competition.h"
#include <pthread.h>
#ifndef PRY_PROCESS_H
#define PRY_PROCESS_H

#define CITY_MUTEX 2
#define HALL_MUTEX 3
#define STATE_MUTEX 4
#define HOTEL_SLOTS_MUTEX 5
#define CLOCK_MUTEX 6
#define COMPETITION_CLOCK_MUTEX 7
#define POTENTIAL_USERS_MUTEX 8
#define SIGNED_USERS_MUTEX 9
#define CITY_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX 10
#define ID_OF_COMPETITION_WE_TAKE_PART_IN_MUTEX 11
#define LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HOTEL_MUTEX 12
#define LIST_OF_PROCESSES_WANTING_PLACE_IN_OUR_HALL_MUTEX 13
#define HOTEL_AGREED_MUTEX 14
#define HALL_AGREED_MUTEX 15

struct hotelRequestStruct {
    //recv[0] = clock, recv[1] = city, recv[2] = competitionClock, recv[3] = competitionId
    int processOfRequest;
    int clockOfRequest;
    int cityOfRequest;
    int competitionClockOfRequest;
    int competitionIdOfRequest;
};

struct structToSend {
    long numberOfCities;
    long numberOfHalls;
    long numberOfRoomsInHotel;
    int rank;
    int size;
    long city;
    long hall;
    int state;
    int clock;
    int competitionClock;
    vector<int> potentialUsers;
    vector<int> signedUsers;
    int cityOfCompetitionWeTakePartIn;
    int idOfCompetitionWeTakePartIn;
    vector<int> listOfProcessesWantingPlaceInOurHotel;
    vector<int> listOfProcessesWantingPlaceInOurHall;
    int hotelAgreed;
    int hallAgreed;
    int hotelRequestClock;
    int hallRequestClock;
    vector<int> lastHotelRequestFromProcessesList;
    vector<hotelRequestStruct> vectorOfHotelRequestsToRespond;
};

class Process {

private:
    structToSend str;
    static void *doYouOrganizeResponder(void *ptr);
    static void *someoneOrganisesResponder(void *ptr);
    static void *canIHavePlaceInHotelResponder(void *ptr);
    static void *canITakeTheHallResponder(void *ptr);
    static bool freeSlotInVectors(structToSend* str);
    void sendMessagesAskingIfCompetitionIsHeld();
    void sendMessagesAskingHotel();
    void sendMessagesAskingHall();
    void printInfo(string info);
    static void printInfoFromThread(string info, structToSend *str);
    void clearStructure(structToSend str);
    string getState(int state);
    static string getStateFromThread(structToSend *str);
public:
    Process();
    void behaviour();


    Process(long i, long i1, long i2);

    vector<int> randomize(structToSend send);



};


#endif //PRY_PROCESS_H
