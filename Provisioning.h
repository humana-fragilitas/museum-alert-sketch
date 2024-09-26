#include<Arduino.h>
#include <Preferences.h>

#include "Configuration.h"

#ifndef PROVISIONING
#define PROVISIONING

class Provisioning {

  private:
    bool isRegistered;
    void onCertificates();

  public:
    bool addDevice(ConnectionSettings settings);

};

#endif

