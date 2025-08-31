#include "device_controls.h"

// Global counter to track ISR calls
volatile int isrCallCount = 0;

void DeviceControls::initialize() noexcept{

  resetButtonInterval = Timing::RESET_BUTTON_HOLD_TIME_MS;
  
  // Remove pinMode - it might be interfering with the original setup
  // pinMode(Pins::ResetButton, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);
  DEBUG_PRINTLN("DeviceControls initialized");

};

void IRAM_ATTR DeviceControls::onResetButtonISR() noexcept {

  unsigned long currentMillis = millis();

  if (digitalRead(Pins::ResetButton)) {
    DeviceControls::previousResetButtonInterval = currentMillis;

  } else {
    if (currentMillis - DeviceControls::previousResetButtonInterval >=
      DeviceControls::resetButtonInterval) {

      // Erase NVS and restart - this replicates the original reset() functionality
      nvs_flash_deinit();
      nvs_flash_erase();
      nvs_flash_init();
      
      // Restart the device
      esp_restart();

    }

  }

}

void DeviceControls::process() {

  static unsigned long lastProcessCall = 0;
  static int lastIsrCount = 0;
  unsigned long currentMillis = millis();
  
  // Show that process is being called every 5 seconds
  if (currentMillis - lastProcessCall >= 5000) {
    DEBUG_PRINTF("Process called, shouldReset: %d, ISR calls: %d\n", shouldReset, isrCallCount);
    lastProcessCall = currentMillis;
  }

  if (shouldReset) {
    DEBUG_PRINTLN("Process: shouldReset flag detected, calling reset()...");
    shouldReset = false;
    reset();
  }

}

void DeviceControls::reset() {

  DEBUG_PRINTLN("Resetting and restarting device...");

  // Deinitialize NVS first
  nvs_flash_deinit();
  
  // Erase NVS partition
  esp_err_t err = nvs_flash_erase();
  if (err != ESP_OK) {
    DEBUG_PRINTF("NVS erase failed: %s\n", esp_err_to_name(err));
  }
  
  // Reinitialize NVS
  err = nvs_flash_init();
  if (err != ESP_OK) {
    DEBUG_PRINTF("NVS init failed: %s\n", esp_err_to_name(err));
  }

  // Small delay to ensure operations complete
  delay(100);
  
  // Force restart
  ESP.restart();

}

volatile bool DeviceControls::shouldReset = false;
unsigned long DeviceControls::previousResetButtonInterval = 0;
unsigned long DeviceControls::resetButtonInterval = 0;