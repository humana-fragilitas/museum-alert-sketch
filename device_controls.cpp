#include "device_controls.h"

void DeviceControls::initialize() {

  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);

};

void IRAM_ATTR DeviceControls::onResetButtonISR() {

  unsigned long currentMillis = millis();

   if (digitalRead(Pins::ResetButton)) {

    DeviceControls::previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - DeviceControls::previousResetButtonInterval >= DeviceControls::resetButtonInterval) {

      DEBUG_PRINTLN("Reset button pressed...");
      DEBUG_PRINTLN("Erasing AP settings and rebooting...");

      // avoids direct call to reset() method here:
      // as it contains ISR-unsafe code
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

  // nvs_flash_erase();
  // nvs_flash_init();

  // ESP.restart();

  //wiFiManager.disconnect(true, false);
  //WiFi.eraseAP();
  // esp_wifi_start();
  // Note: first restart after serial flashing causes puts the board in boot mode:(1,7) (purple led)
  // https://github.com/esp8266/Arduino/issues/1722
  // https://github.com/esp8266/Arduino/issues/1017
  // https://github.com/esp8266/Arduino/issues/1722#issuecomment-321818357

}

volatile bool DeviceControls::shouldReset = false;
unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;