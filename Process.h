//
// Created by sefir on 06.06.17.
//
#include <string>
#include <mutex>
#include "Competition.h"
#include <pthread.h>
#ifndef PRY_PROCESS_H
#define PRY_PROCESS_H

#define RANK_MUTEX 0
#define SIZE_MUTEX 1
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


struct structToSend {
    int rank;
    int size;
    int city;
    int hall;
    int state;
    int hotelSlots;
    vector<int> clock;
    vector<int> competitionClock;
    vector<int> potentialUsers;
    vector<int> signedUsers;
    int cityOfCompetitionWeTakePartIn;
    int idOfCompetitionWeTakePartIn;

};

class Process {

private:
    structToSend str;
    static void *askIfCompetitionIsHeld(void *ptr);
    static void *doYouOrganizeResponder(void *ptr);
    static void *someoneOrganisesResponder(void *ptr);
    static bool freeSlotInVectors(structToSend* str);
    void sendMessagesAskingIfCompetitionIsHeld(structToSend str);
    void sendMessagesAskingHotel(structToSend str);

public:
    static const int DO_YOU_CREATE_A_COMPETITION;
    Process();
    void behaviour();

    void receiveRepliesToQuestionAboutHavingACompetition();
};


#endif //PRY_PROCESS_H
