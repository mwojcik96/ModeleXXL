//
// Created by sefir on 07.06.17.
//

#include "Agent.h"

string Agent::generateRoll() {
    int randomNumber = rand()%100+1;
    printf("%d\n", randomNumber);
    if (randomNumber < 20) {
        return ORGANIZER;
    }
    return PARTICIPANT;
}

void Agent::setRole(string role) {
    this->role = role;
}

string Agent::getRole() {
    return this->role;
}

void Agent::setGeneratingRole(bool i) {
    this->toGenerateRole = i;
}

bool Agent::getGeneratingRole() {
    return this->toGenerateRole;
}
