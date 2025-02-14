#ifndef SERIAL_COM
#define SERIAL_COM

#include<Arduino.h>
#include<ArduinoJson.h>

#include "settings.h"
#include "macros.h"

class SerialCom {

  public:
    static USBCDC initializeSerial(unsigned const int waitTime = Timing::SERIAL_PORT_INIT_TIMEOUT_MS);
    static void sendAvailableWiFiNetworks(String wifiScanJson);
    static ProvisioningSettings getProvisioningSettings();

};

#endif