#include <Arduino.h>

#include "Configuration.h"
#include "macros.h"

#ifndef LED_INDICATORS
#define LED_INDICATORS

class LedIndicators {

  private:
    static TaskHandle_t taskHandle;

  public:
    static void setState(AppState appState, bool isWiFiConnected, bool isMqttBrokerConnected);

};

#endif