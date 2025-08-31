#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

#include <Arduino.h>
#include <esp_system.h>
#include <esp_err.h>
#include <WiFi.h>
// erase non-volatile storage
#include "nvs_flash.h"

#include "config.h"
#include "storage_manager.h"

class DeviceControls {

  private:
    static volatile bool shouldReset;
    static unsigned long previousResetButtonInterval;
    static unsigned long resetButtonInterval;
    static void onResetButtonISR();

  public:
    static void initialize();
    static void process();
    static void reset();

};

#endif