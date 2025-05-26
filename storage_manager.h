#ifndef CERT_MANAGER
#define CERT_MANAGER

#include <Arduino.h>
#include <Preferences.h>

#include "macros.h"
#include "settings.h"
#include "ciphering.h"

using Distance = float;

class StorageManager {

  public:

    template<typename T>
    static T load();

    template<typename T>
    static bool save(const T& value);

    static void erase();

};

#endif