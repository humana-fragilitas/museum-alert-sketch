#ifndef DEVICE_CONTROLS
#define DEVICE_CONTROLS

#include <Arduino.h>
#include <esp_system.h>
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
   static void onResetButtonISR() noexcept;

  public:
   static void initialize() noexcept;
   static void process() noexcept;
   [[noreturn]] static void reset() noexcept;

   // Disable copy constructor and assignment operator
   DeviceControls() = delete;
   DeviceControls(const DeviceControls&) = delete;
   DeviceControls& operator=(const DeviceControls&) = delete;

};

#endif