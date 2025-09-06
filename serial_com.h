#ifndef SERIAL_COM
#define SERIAL_COM

#include<Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "sensor.h"


void initializeSerial();

class SerialCom {

  private:
   static bool detectMarker(char rc, const char marker[], int& matchCount) noexcept;

  public:
   static void initialize(unsigned const int timeout = Timing::SERIAL_PORT_INIT_TIMEOUT_MS) noexcept;
   static void send(USBMessageType type, const String& cid, JsonVariant payload);
   static void error(ErrorType type, const String& correlationId = "");
   static void acknowledge(const String& correlationId);
   static String getStringWithMarkers();
   static RequestWrapper waitForRequest();
   static WiFiCredentials receiveWiFiCredentials();
   static Certificates receiveProvisioningCertificates();

};

#endif