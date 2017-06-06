//
// Created by sefir on 06.06.17.
//
#include <vector>
using namespace std;
#ifndef PRY_COMPETITION_H
#define PRY_COMPETITION_H


class Competition {

    int id;
    vector<int> priority;

    // Constructors
    Competition() {};
    Competition(int id);
    Competition(int id, vector<int> vector);


};


#endif //PRY_COMPETITION_H
