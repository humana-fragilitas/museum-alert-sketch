#include "led_indicators.h"

void LedIndicators::setState(AppState currentAppState,
                             bool hasWiFiConnection,
                             bool hasMqttBrokerConnection,
                             bool hasAlarm) noexcept {

  appState = currentAppState;
  isWiFiConnected = hasWiFiConnection;
  isMqttBrokerConnected = hasMqttBrokerConnection;
  isAlarmActive = hasAlarm;

}

void LedIndicators::initialize() noexcept {

  if (ledBlinkingTaskHandle == nullptr) {
    xTaskCreate(LedIndicators::ledBlinkingTask, "LED_INDICATORS", 1024, nullptr, 1, &ledBlinkingTaskHandle);
  }

}

void LedIndicators::ledBlinkingTask(void *pvParameters) {

  for (;;) {

    const unsigned long currentMillis = millis();

    onEveryMS(currentMillis, FAST_INTERVAL, [] {
      digitalWrite(Pins::WiFi, isWiFiConnected);
      digitalWrite(Pins::Alarm, isAlarmActive);
    });

    switch (appState) {

      case AppState::PROVISION_DEVICE:
        onEveryMS(currentMillis, FAST_INTERVAL, [] {
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      case AppState::CONFIGURE_WIFI:
      case AppState::CONFIGURE_CERTIFICATES:
        onEveryMS(currentMillis, MEDIUM_INTERVAL, [] {
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      case AppState::DEVICE_INITIALIZED:
        digitalWrite(Pins::Status, HIGH);
        break;
      case AppState::FATAL_ERROR:
        onEveryMS(currentMillis, FASTEST_INTERVAL, [] {
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      default:
        onEveryMS(currentMillis, SLOW_INTERVAL, [] {
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

TaskHandle_t LedIndicators::ledBlinkingTaskHandle{ nullptr };
AppState LedIndicators::appState{ AppState::STARTED };
bool LedIndicators::isWiFiConnected{ false };
bool LedIndicators::isAlarmActive{ false };
bool LedIndicators::isMqttBrokerConnected{ false };