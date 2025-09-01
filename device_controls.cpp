#include "device_controls.h"

void DeviceControls::initialize() noexcept{

  resetButtonInterval = Timing::RESET_BUTTON_HOLD_TIME_MS;
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

        DeviceControls::reset();

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

unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;
volatile int isrCallCount = 0;