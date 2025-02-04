#include <Arduino.h>
#include <Preferences.h>

#include "macros.h"
#include "Configuration.h"
#include "Ciphering.h"

#ifndef CERT_MANAGER
#define CERT_MANAGER

class CertManager {

  private:
    Preferences preferences;
    static const unsigned char DSTroot_CA[];
    static const char storageNamespace[];
    static const char clientCertStorageLabel[];
    static const char privateKeyStorageLabel[];
    static const char aesKey[];
    static const char aesIV[];

  public:
    void storeCertificates(Certificates certificates);
    Certificates retrieveCertificates();

};

#endif