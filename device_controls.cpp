#include "device_controls.h"


void DeviceControls::initialize() noexcept{

  resetButtonInterval = Timing::RESET_BUTTON_HOLD_TIME_MS;
  
  xTaskCreate(
    resetTask,
    "ResetTask",
    2048,
    NULL,
    2,
    &resetTaskHandle
  );
  
  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);
  DEBUG_PRINTLN("DeviceControls initialized");

};

void IRAM_ATTR DeviceControls::onResetButtonISR() noexcept {

  const unsigned long currentMillis = millis();

  if (digitalRead(Pins::ResetButton)) {
    DeviceControls::previousResetButtonInterval = currentMillis;

  } else {
    if (currentMillis - DeviceControls::previousResetButtonInterval >=
      DeviceControls::resetButtonInterval) {

        DeviceControls::resetRequested = true;

    }

  }

}

void DeviceControls::reset() {

  DEBUG_PRINTLN("Resetting and restarting device...");

    nvs_flash_deinit();
    nvs_flash_erase();
    nvs_flash_init();
    esp_restart();

}

void DeviceControls::resetTask(void* parameter) {
  while (true) {
    if (resetRequested) {
      resetRequested = false;
      DEBUG_PRINTLN("Reset requested from ISR, executing reset...");
      reset();
    }
    // Check every 10ms for reset requests
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;
volatile bool DeviceControls::resetRequested = false;
TaskHandle_t DeviceControls::resetTaskHandle = NULL;
volatile int isrCallCount = 0;