#include <Arduino.h>
#include <Preferences.h>

#include "Macros.h"
#include "Configuration.h"
#include "Ciphering.h"

#ifndef CERT_MANAGER
#define CERT_MANAGER

class CertManager {

  private:
    static const unsigned char DSTroot_CA[];
    static const char storageNamespace[];
    static const char clientCertStorageLabel[];
    static const char privateKeyStorageLabel[];
    static const char aesKey[];
    static const char aesIV[];

  public:
    static void storeCertificates(Certificates certificates);
    static void eraseCertificates();
    static Certificates retrieveCertificates();

};

#endif