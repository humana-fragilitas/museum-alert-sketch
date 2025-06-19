#ifndef CERT_MANAGER
#define CERT_MANAGER

#include <Arduino.h>
#include <Preferences.h>

#include "config.h"
#include "ciphering.h"

using Distance = float;
using BeaconURL = String;

class StorageManager {

  public:

    template<typename T>
    static T load();

    template<typename T>
    static bool save(const T& value);

    static void erase();

};

#endif