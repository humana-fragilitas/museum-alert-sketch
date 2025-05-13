#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <ArduinoJson.h>

#include "settings.h"

class JsonHelper {
public:
  static Certificates parseProvisioningCertificates(const String& json);
  static WiFiCredentials parseWiFiCredentials(const String& json);
  static USBCommandType parseUSBCommand(const String& jsonString);
};

#endif // JSON_HELPER_H