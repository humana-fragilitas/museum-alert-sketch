#include "helpers.h"

std::vector<callbackEntry> callbackEntries;
std::vector<callback> onceCallbackEntries;

void once(callback cbFunction) {

  for (const auto& existingCallback : onceCallbackEntries) {
   if (existingCallback == cbFunction) {
     return;
   }
  }
  
  onceCallbackEntries.push_back(cbFunction);
  cbFunction();

}

void onEveryMS(unsigned int currentMillis, unsigned int everyMillis, callback cbFunction, bool isImmediate) {

  int index = -1;

  for (auto i = 0u; i < callbackEntries.size(); ++i) {
   if (callbackEntries[i].callbackFunction == cbFunction) {
     index = static_cast<int>(i);
     break;
   }
  }

  if (index != -1) {
   auto& entry = callbackEntries[static_cast<size_t>(index)];
   if ((currentMillis - entry.prevMillis) >= entry.everyMillis) {
     entry.callbackFunction();
     entry.prevMillis = currentMillis;
   }
  } else {
   callbackEntry tempCallback{everyMillis, currentMillis, cbFunction};
   callbackEntries.push_back(tempCallback);
   if (isImmediate) cbFunction();
  }

}