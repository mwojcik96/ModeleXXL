//
// Created by sefir on 06.06.17.
//
#include <string>
#include "Competition.h"
#ifndef PRY_PROCESS_H
#define PRY_PROCESS_H

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
};

class Process {
private:

    structToSend str;
    static void *askIfCompetitionIsHeld(void *ptr);
    static void *organizationResponder(void *ptr);
    static bool freeSlotInVectors(vector<int> potentialUsers, vector<int> signedUsers, int hotelSlots);

public:
    static const int DO_YOU_CREATE_A_COMPETITION;
    Process();
    void behaviour();
};


#endif //PRY_PROCESS_H
