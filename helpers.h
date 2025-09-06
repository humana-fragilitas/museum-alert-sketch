#ifndef HELPERS
#define HELPERS

#include <Arduino.h>
#include <vector>


using callback = void (*)(void);
struct callbackEntry {
  unsigned int everyMillis;
  unsigned int prevMillis;
  callback callbackFunction;
};

void once(callback cbFunction);
void onEveryMS(unsigned int currentMillis, unsigned int everyMillis, callback cbFunction, bool isImmediate = true);

#endif