//
// Created by sefir on 06.06.17.
//

#include "Competition.h"


Competition::Competition(int id) {
    this->id = id;
}

Competition::Competition(int id, vector<int> vector) {
    this->id = id;
    for(int i = 0; i < vector.size(); i++) {
        this->priority.push_back(vector[i]);
    }
}
