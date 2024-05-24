#include <iostream>
#include <vector>
#include <functional>

#include "helpers.h"

std::vector<callbackEntry> callbackEntries;
std::vector<callback> onceCallbackEntries;

void once(callback cbFunction) {

    for (size_t i = 0; i < onceCallbackEntries.size(); ++i) {
      if (onceCallbackEntries[i] == cbFunction) {
          return;
      }
    }
    
    onceCallbackEntries.push_back(cbFunction);
    cbFunction();

}

void onEveryMS(unsigned int currentMillis, unsigned int everyMillis, callback cbFunction) {

    int index = -1;

    for (size_t i = 0; i < callbackEntries.size(); ++i) {
        if (callbackEntries[i].callbackFunction == cbFunction) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        if ((currentMillis - callbackEntries[index].prevMillis) >= callbackEntries[index].everyMillis) {
            callbackEntries[index].callbackFunction();
            callbackEntries[index].prevMillis = currentMillis;
        }
    } else {
        // Add new function object to the vector
        callbackEntry tempCallback = {everyMillis, currentMillis, cbFunction};
        callbackEntries.push_back(tempCallback);
    }

}