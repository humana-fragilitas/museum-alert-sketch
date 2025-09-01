#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

#include <Arduino.h>
#include <esp_system.h>
#include <esp_err.h>
#include <WiFi.h>
#include "nvs_flash.h"

#include "config.h"
#include "storage_manager.h"

class DeviceControls {

  private:
    static unsigned long previousResetButtonInterval;
    static unsigned long resetButtonInterval;
    static void onResetButtonISR() noexcept;

  public:
    static void initialize() noexcept;
    static void reset();
  };

#endif