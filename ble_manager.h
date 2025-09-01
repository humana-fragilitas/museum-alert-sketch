#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

#include "sensor.h"
#include "config.h"

class BLEManager {

public:
  static void initialize() noexcept;
  static void startBeacon(const String& url) noexcept;
  static void stopBeacon() noexcept;
  static void maintainBeacon() noexcept;
  static void cleanup() noexcept;
  static bool isBeaconActive() noexcept;

private:
  static bool initialized;
  static bool beaconActive;
  static String currentUrl;
  static BLEAdvertising* pAdvertising;
  static unsigned long lastMaintenanceTime;

  static String encodeUrl(const String& url);
  static uint8_t getUrlSchemePrefix(const String& url) noexcept;
  static String compressUrl(const String& url);
  
};

#endif
