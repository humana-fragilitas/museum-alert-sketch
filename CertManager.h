#include <Arduino.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_tls.h"

#include "Configuration.h"

class CertManager {

  private:
    static const unsigned char DSTroot_CA[];

  public:
    static void storeCertificates(Certificates certificates);
    static Certificates retrieveCertificates();

};