#include <ArduinoBLE.h>
#include <ArduinoJson.h>

#include "Macros.h"
#include "Sensor.h"
#include "Configuration.h"

#ifndef BLE_MANAGER
#define BLE_MANAGER

class BLEManager {

  private:
    static const char* deviceServiceUuid;
    static const char* deviceServiceConfigurationCharacteristicUuid;
    static const char* deviceServiceSsidsCharacteristicUuid;
    static BLEService configurationService;
    static BLEStringCharacteristic wiFiSsidsCharacteristic;
    static BLEStringCharacteristic configurationCharacteristic;
    void(*_onWiFiCredentials)(String);
    void(*_onTLSCertificate)(String);

  public:
    BLEManager();
    bool initializeDeviceConfigurationService();
    /* std::pair<WiFiCredentials, ConnectionSettings> */ ProvisioningSettings getDeviceConfiguration(String json);
    void configureViaBLE();
    bool disconnect();

};

#endif