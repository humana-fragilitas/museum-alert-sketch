#include "device_controls.h"

void DeviceControls::initialize() noexcept {

  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);

};

void IRAM_ATTR DeviceControls::onResetButtonISR() noexcept {

  const auto currentMillis = millis();

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

void DeviceControls::process() noexcept {

  if (shouldReset) {
   shouldReset = false;
   reset();
  }

}

[[noreturn]] void DeviceControls::reset() noexcept {

  DEBUG_PRINTLN("Resetting and restarting device...");

  nvs_flash_erase();
  nvs_flash_init();

  ESP.restart();
  
  // This should never be reached, but satisfies the compiler's noreturn requirement
  while (true) {
   // Infinite loop to ensure function never returns
  }

}

volatile bool DeviceControls::shouldReset = false;
unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;