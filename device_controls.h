#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

#include <Arduino.h>
#include <esp_system.h>
#include <esp_err.h>
#include <WiFi.h>
#include "nvs_flash.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config.h"
#include "storage_manager.h"


class DeviceControls {

  private:
    static unsigned long previousResetButtonInterval;
    static unsigned long resetButtonInterval;
    static volatile bool resetRequested;
    static TaskHandle_t resetTaskHandle;
    static void onResetButtonISR() noexcept;
    static void resetTask(void* parameter);

  public:
    static void initialize() noexcept;
    static void reset();
  };

#endif