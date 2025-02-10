#include "Helpers.h"

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
        callbackEntry tempCallback = {everyMillis, currentMillis, cbFunction};
        callbackEntries.push_back(tempCallback);
    }

}

void encryptionKeyToHexString(const std::vector<uint8_t>& key, char* outputBuffer, size_t bufferSize) {

  if (!outputBuffer || bufferSize < (key.size() * 2 + 1)) { 
      return; // Prevent buffer overflow
  }

  size_t pos = 0;
  for (uint8_t byte : key) {
      if (pos < bufferSize - 2) { // Ensure space for two hex chars + null terminator
          snprintf(&outputBuffer[pos], 3, "%02X", byte);
          pos += 2;
      } else {
          break; // Avoid buffer overflow
      }
  }

  outputBuffer[pos] = '\0'; // Null-terminate the string
  
}