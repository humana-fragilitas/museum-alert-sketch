#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

#include <Arduino.h>
#include <esp_system.h>
#include <WiFi.h>

#include "macros.h"
#include "pins.h"
#include "cert_manager.h"

class DeviceControls {

  private:
    static unsigned long previousResetButtonInterval;
    static unsigned long resetButtonInterval;
    static void onResetButtonISR();

  public:
    static void initialize();
    static void reset();

};

#endif