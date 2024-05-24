#include <iostream>
#include <vector>
#include <chrono>
#include <functional>
#include <thread>

#ifndef HELPERS
#define HELPERS

typedef void (*callback)(void);
struct callbackEntry {
    unsigned int everyMillis;
    unsigned int prevMillis;
    callback callbackFunction;
};

void once(callback cbFunction);
void onEveryMS(unsigned int currentMillis, unsigned int everyMillis, callback cbFunction);

#endif