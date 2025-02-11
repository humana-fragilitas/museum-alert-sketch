#include "led_indicators.h"

void LedIndicators::setState(AppState appState, bool isWiFiConnected, bool isMqttBrokerConnected, bool hasAlarm) {

  m_appState = appState;
  m_isWiFiConnected = isWiFiConnected;
  m_isMqttBrokerConnected = isMqttBrokerConnected;
  m_hasAlarm = hasAlarm;

};

void LedIndicators::initialize(void) {

  if (ledBlinkingTaskHandle == nullptr) {
    xTaskCreate(LedIndicators::ledBlinkingTask, "LED_INDICATORS", 4096, nullptr, 1, &ledBlinkingTaskHandle);
  }

};

void LedIndicators::ledBlinkingTask(void *pvParameters) {

  for(;;) {

    unsigned long currentMillis = millis();

    onEveryMS(currentMillis, FAST_INTERVAL, []{
      digitalWrite(Pins::WiFi, m_isWiFiConnected);
      // digitalWrite(Pins::Mqtt, m_isWiFiConnected); // TO DO: add mqtt led indicator
      digitalWrite(Pins::Alarm, m_hasAlarm);
    });

    switch(m_appState) {

      case PROVISION_DEVICE:
        onEveryMS(currentMillis, FAST_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      case CONFIGURE_DEVICE:
        onEveryMS(currentMillis, MEDIUM_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      [[fallthrough]]
      case INITIALIZE_BLE:
      default:
        onEveryMS(currentMillis, SLOW_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });

    }

    vTaskDelay(pdMS_TO_TICKS(10));
    
  }

};

TaskHandle_t LedIndicators::ledBlinkingTaskHandle = nullptr;
AppState LedIndicators::m_appState = AppState::STARTED;
bool LedIndicators::m_isWiFiConnected = false;
bool LedIndicators::m_hasAlarm = false;
bool LedIndicators::m_isMqttBrokerConnected = false;