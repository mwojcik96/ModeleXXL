//
// Created by sefir on 07.06.17.
//

#ifndef PRY_AGENT_H
#define PRY_AGENT_H
#define ORGANIZER "Organizer"
#define PARTICIPANT "Participant"

#include <string>
#include "Competition.h"

class Agent {
    // It can be either ORGANIZER or PARTICIPANT
    string role;
    // It's object of the competition which we organize or which we will take part in
    Competition competition;
    // When this field is true, agent will randomly choose a role
    bool toGenerateRole;

public:

    void setRole(string role);

    string getRole();

    static string generateRoll();

    void setGeneratingRole(bool i);

    bool getGeneratingRole();
};


#endif //PRY_AGENT_H
