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

String encryptionKeyToHexString(const std::vector<uint8_t>& key) {
  
  String hexString;
  for (size_t i = 0; i < key.size(); ++i) {
    if (key[i] < 0x10) {
        hexString += "0"; // Add leading zero for single-digit hex values
    }
    hexString += String(key[i], HEX);
  }
  return hexString;
  
}