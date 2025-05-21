#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <ArduinoJson.h>

#include "settings.h"

class JsonHelper {
public:
  template<typename T>
  static T parse(const String& json);
};

template<> 
WiFiCredentialsRequest JsonHelper::parse<WiFiCredentialsRequest>(const String& json);
  
template<> 
CertificatesRequest JsonHelper::parse<CertificatesRequest>(const String& json);  

template<> 
DeviceCommandRequest JsonHelper::parse<DeviceCommandRequest>(const String& json);

#endif // JSON_HELPER_H