//
// Created by sefir on 06.06.17.
//

#include "Process.h"

void Process::behaviour() {

}

static string Process::generateRoll() {
    int randomNumber = rand()%100+1;
    if (randomNumber < 15) {
        return ORGANIZER;
    }
    return PARTICIPANT;
}
