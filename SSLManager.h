#include <Arduino.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_tls.h"

class SSLManager {

  private:
    static const unsigned char DSTroot_CA[] ;

  public:
    void setCertificate();

};