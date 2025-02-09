#include <Arduino.h>
#include <Preferences.h>

#include "Macros.h"
#include "Configuration.h"
#include "Ciphering.h"

#ifndef CERT_MANAGER
#define CERT_MANAGER

class CertManager {

  public:
    static void storeCertificates(Certificates certificates);
    static void eraseCertificates();
    static Certificates retrieveCertificates();

};

#endif