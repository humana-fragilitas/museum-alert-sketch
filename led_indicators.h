#ifndef LED_INDICATORS
#define LED_INDICATORS

#include <Arduino.h>

#include "config.h"
#include "helpers.h"
#include "macros.h"

class LedIndicators {

  private:
    static constexpr unsigned long SLOW_INTERVAL = 500;
    static constexpr unsigned long MEDIUM_INTERVAL = 250;
    static constexpr unsigned long FAST_INTERVAL = 125;
    static TaskHandle_t ledBlinkingTaskHandle;
    static void ledBlinkingTask(void *pvParameters);
    static AppState appState;
    static bool isWiFiConnected;
    static bool isMqttBrokerConnected;
    static bool isAlarmActive;

  public:
    static void initialize(void);
    static void setState(AppState currentAppState, bool hasWiFiConnection, bool hasMqttBrokerConnection, bool hasAlarm);

};

#endif