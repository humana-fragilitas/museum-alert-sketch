#ifndef BLE_MANAGER
#define BLE_MANAGER

#include <ArduinoBLE.h>
#include <ArduinoJson.h>

#include "macros.h"
#include "sensor.h"


class BLEManager {

  private:
    static const char* deviceServiceUuid;
    static const char* deviceServiceConfigurationCharacteristicUuid;
    static const char* deviceServiceSsidsCharacteristicUuid;
    static BLEService configurationService;
    static BLEStringCharacteristic wiFiSsidsCharacteristic;
    static BLEStringCharacteristic configurationCharacteristic;

  public:
    BLEManager();
    bool initializeDeviceConfigurationService();
    ProvisioningSettings getDeviceConfiguration(const char *json);
    void configureViaBLE();
    bool disconnect();

};

#endif