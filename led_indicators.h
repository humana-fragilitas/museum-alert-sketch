#ifndef LED_INDICATORS
#define LED_INDICATORS

#include <Arduino.h>

#include "config.h"
#include "helpers.h"

class LedIndicators {

  private:
    static constexpr unsigned long SLOW_INTERVAL = 520;
    static constexpr unsigned long MEDIUM_INTERVAL = 260;
    static constexpr unsigned long FAST_INTERVAL = 130;
    static constexpr unsigned long FASTEST_INTERVAL = 75;
    static TaskHandle_t ledBlinkingTaskHandle;
    static void ledBlinkingTask(void *pvParameters);
    static AppState appState;
    static bool isWiFiConnected;
    static bool isMqttBrokerConnected;
    static bool isAlarmActive;

  public:
    static void initialize() noexcept;
    static void setState(AppState currentAppState, bool hasWiFiConnection, bool hasMqttBrokerConnection, bool hasAlarm) noexcept;
  };

#endif