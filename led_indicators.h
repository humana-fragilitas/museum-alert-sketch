#ifndef LED_INDICATORS
#define LED_INDICATORS

#include <Arduino.h>

#include "settings.h"
#include "pins.h"
#include "helpers.h"
#include "macros.h"

class LedIndicators {

  private:
    static constexpr unsigned long SLOW_INTERVAL = 500;
    static constexpr unsigned long MEDIUM_INTERVAL = 250;
    static constexpr unsigned long FAST_INTERVAL = 125;
    static TaskHandle_t ledBlinkingTaskHandle;
    static void ledBlinkingTask(void *pvParameters);
    static AppState m_appState;
    static bool m_isWiFiConnected;
    static bool m_isMqttBrokerConnected;
    static bool m_hasAlarm;

  public:
    static void initialize(void);
    static void setState(AppState appState, bool isWiFiConnected, bool isMqttBrokerConnected, bool hasAlarm);

};

#endif