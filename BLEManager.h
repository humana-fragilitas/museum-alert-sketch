#include <ArduinoBLE.h>

#include "UserPreferences.h"

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
    BLEManager(void(*_onWiFiCredentials)(String), void(*_onTLSCertificate)(String));
    void initializeBLEConfigurationService();
    void configureWiFi(String json);
    void configureViaBLE();

};

#endif