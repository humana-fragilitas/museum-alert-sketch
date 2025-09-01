#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <ArduinoJson.h>

#include "config.h"

class JsonHelper {
  public:
    template<typename T>
    static T parse(const String& json);
};

template<>
WiFiCredentials JsonHelper::parse<WiFiCredentials>(const String& json);

template<>
Certificates JsonHelper::parse<Certificates>(const String& json);

#endif // JSON_HELPER_H