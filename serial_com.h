#ifndef SERIAL_COM
#define SERIAL_COM

#include<Arduino.h>

#include "settings.h"

void initializeSerial();

class SerialCom {

  private:
    static bool detectMarker(char rc, const char marker[], int &matchCount);

  public:
    static void initialize(unsigned const int timeout = Timing::SERIAL_PORT_INIT_TIMEOUT_MS);
    static void send(String payload);
    static String getStringWithMarkers();
    static WiFiCredentials receiveWiFiCredentials();
    static Certificates receiveProvisioningCertificates();

};

#endif