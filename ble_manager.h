#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include "config.h"

class BLEManager {
public:
    static void initialize();
    static void startBeacon(const String& url);
    static void stopBeacon();
    static void maintainBeacon();
    static void cleanup();
    static bool isBeaconActive();

private:
    static bool initialized;
    static bool beaconActive;
    static String currentUrl;
    static BLEAdvertising* pAdvertising;
    static unsigned long lastMaintenanceTime;

    static String encodeUrl(const String& url);
    static uint8_t getUrlSchemePrefix(const String& url);
    static String compressUrl(const String& url);
};

#endif
