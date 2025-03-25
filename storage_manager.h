#ifndef CERT_MANAGER
#define CERT_MANAGER

#include <Arduino.h>
#include <Preferences.h>

#include "macros.h"
#include "settings.h"
#include "ciphering.h"

class StorageManager {

  public:
    static bool saveConfiguration(DeviceConfiguration configuration);
    static bool saveDistance(float distance);
    static DeviceConfiguration loadConfiguration();
    static float loadDistance();
    static void erase();

};

#endif