//
// Created by sefir on 06.06.17.
//
#include <string>
#include "Competition.h"
#ifndef PRY_PROCESS_H
#define PRY_PROCESS_H
#define ORGANIZER "Organizer"
#define PARTICIPANT "Participant"

class Process {
private:
    // It can be either ORGANIZER or PARTICIPANT
    string role;
    // It's object of the competition which we organize or which we will take part in
    Competition competition;

    int rank;
    int size;
    static void *askIfCompetitionIsHeld(void *ptr);

public:
    static const int DO_YOU_CREATE_A_COMPETITION;
    Process();
    void behaviour();
    static string generateRoll();
};


#endif //PRY_PROCESS_H
