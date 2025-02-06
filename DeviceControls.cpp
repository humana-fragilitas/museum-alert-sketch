#include "DeviceControls.h"

void DeviceControls::initialize(void) {

  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);

};

void DeviceControls::onResetButtonISR(void) {

  unsigned long currentMillis = millis();

   if (digitalRead(Pins::ResetButton)) {

    DeviceControls::previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - DeviceControls::previousResetButtonInterval >= DeviceControls::resetButtonInterval) {

      DEBUG_PRINTLN("Reset button pressed...");
      DEBUG_PRINTLN("Erasing AP settings and rebooting...");
      WiFi.eraseAP();
      ESP.restart();

      //wiFiManager.disconnect(true, false);
      //WiFi.eraseAP();
      // esp_wifi_start();
      // Note: first restart after serial flashing causes puts the board in boot mode:(1,7) (purple led)
      // https://github.com/esp8266/Arduino/issues/1722
      // https://github.com/esp8266/Arduino/issues/1017
      // https://github.com/esp8266/Arduino/issues/1722#issuecomment-321818357


    }

  }

}

unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;