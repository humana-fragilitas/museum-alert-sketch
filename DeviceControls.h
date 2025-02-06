#include <Arduino.h>
#include <esp_system.h>
#include <WiFi.h>

#include "macros.h"
#include "Pins.h"

#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

class DeviceControls {

  private:
    static unsigned long previousResetButtonInterval;
    static unsigned long resetButtonInterval;
    static void onResetButtonISR(void);

  public:
    static void initialize(void);

};

#endif