#include <Arduino.h>

#include <vector>
#include <cstdio>

#include "Macros.h"

#ifndef HELPERS
#define HELPERS

using callback = void (*)(void);
struct callbackEntry {
    unsigned int everyMillis;
    unsigned int prevMillis;
    callback callbackFunction;
};

void once(callback cbFunction);
void onEveryMS(unsigned int currentMillis, unsigned int everyMillis, callback cbFunction);
String encryptionKeyToHexString(const std::vector<uint8_t>& key);

#endif