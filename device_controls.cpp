#include "device_controls.h"

void DeviceControls::initialize() {

  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);

};

void IRAM_ATTR DeviceControls::onResetButtonISR() {

  unsigned long currentMillis = millis();

   if (digitalRead(Pins::ResetButton)) {

    DeviceControls::previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - DeviceControls::previousResetButtonInterval >=
      DeviceControls::resetButtonInterval) {

      DEBUG_PRINTLN("Reset button pressed: "
                    "erasing all settings and rebooting device...");

      /**
       * Avoids direct call to reset() method here:
       * as it contains ISR-unsafe code
       */
      shouldReset = true;

    }

  }

}

void DeviceControls::process() {

  if (shouldReset) {
    shouldReset = false;
    reset();
  }

}

void DeviceControls::reset() {

  DEBUG_PRINTLN("Resetting and restarting device...");

  nvs_flash_erase();
  nvs_flash_init();

  ESP.restart();

}

volatile bool DeviceControls::shouldReset = false;
unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;