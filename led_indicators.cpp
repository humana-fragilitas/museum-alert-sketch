#include "led_indicators.h"

void LedIndicators::setState(AppState currentAppState,
                             bool hasWiFiConnection,
                             bool hasMqttBrokerConnection,
                             bool hasAlarm) {

  appState = currentAppState;
  isWiFiConnected = hasWiFiConnection;
  isMqttBrokerConnected = hasMqttBrokerConnection;
  isAlarmActive = hasAlarm;

};

void LedIndicators::initialize(void) {

  if (ledBlinkingTaskHandle == nullptr) {
    xTaskCreate(LedIndicators::ledBlinkingTask, "LED_INDICATORS", 1024, nullptr, 1, &ledBlinkingTaskHandle);
  }

};

void LedIndicators::ledBlinkingTask(void *pvParameters) {

  for(;;) {

    unsigned long currentMillis = millis();

    onEveryMS(currentMillis, FAST_INTERVAL, []{
      digitalWrite(Pins::WiFi, isWiFiConnected);
      digitalWrite(Pins::Alarm, isAlarmActive);
    });

    switch(appState) {

      case PROVISION_DEVICE:
        onEveryMS(currentMillis, FAST_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      case CONFIGURE_WIFI:
      case CONFIGURE_CERTIFICATES:
        onEveryMS(currentMillis, MEDIUM_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      case DEVICE_INITIALIZED:
        digitalWrite(Pins::Status, HIGH);
        break;
      case FATAL_ERROR:
        onEveryMS(currentMillis, FASTEST_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
      default:
        onEveryMS(currentMillis, SLOW_INTERVAL, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });

    }

    vTaskDelay(pdMS_TO_TICKS(10));
    
  }

};

TaskHandle_t LedIndicators::ledBlinkingTaskHandle = nullptr;
AppState LedIndicators::appState = AppState::STARTED;
bool LedIndicators::isWiFiConnected = false;
bool LedIndicators::isAlarmActive = false;
bool LedIndicators::isMqttBrokerConnected = false;