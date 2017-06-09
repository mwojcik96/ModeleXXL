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
    vector<int> clock;
    vector<int> competitionClock;
    int city;
    int room;
    int state;
};

class Process {
private:

    int rank;
    int size;
    static void *askIfCompetitionIsHeld(void *ptr);

public:
    static const int DO_YOU_CREATE_A_COMPETITION;
    Process();
    void behaviour();
};


#endif //PRY_PROCESS_H
