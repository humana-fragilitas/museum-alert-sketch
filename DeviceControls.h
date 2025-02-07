#include <Arduino.h>
#include <esp_system.h>
#include <WiFi.h>

#include "Macros.h"
#include "Pins.h"
#include "CertManager.h"

#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

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