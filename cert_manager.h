#ifndef CERT_MANAGER
#define CERT_MANAGER

#include <Arduino.h>
#include <Preferences.h>

#include "macros.h"
#include "settings.h"
#include "ciphering.h"

class CertManager {

  public:
    static bool store(Certificates certificates);
    static void eraseCertificates();
    static Certificates retrieveCertificates();

    static bool store(DeviceConfiguration configuration);
    static void erase();
    static DeviceConfiguration retrieve();

};

#endif