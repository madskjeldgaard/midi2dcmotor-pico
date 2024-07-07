// RandGen.h
#ifndef RAND_GEN_H
#define RAND_GEN_H

#include <cstdlib> // for rand() and srand()
#include <ctime> // for time()

class RandGen {
public:
    RandGen() {
        // Seed the random number generator with the current time
        srand(time(NULL));
    }

    int getRandomInt(int min, int max) {
        // Generate a random integer between min and max
        return min + rand() % (max - min + 1);
    }
};

#endif // RAND_GEN_H
