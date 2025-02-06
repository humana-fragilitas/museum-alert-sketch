#include <mqtt_client.h>
#include <esp_tls.h>
#include <esp_heap_caps.h>

#include "macros.h"
#include "helpers.h"
#include "Pins.h"
#include "PinSetup.h"
#include "SerialCom.h"
#include "Configuration.h"
#include "CertManager.h"
#include "Provisioning.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "BLEManager.h"
#include "LedIndicators.h"
#include "DeviceControls.h"

// Client ID: MAS-EC357A188534

// Mutex mutex;
TaskHandle_t ledIndicatorsTask;

AppState appState,lastAppState;
AlarmPayload payload;

BLEManager bleManager;
CertManager certManager;
ProvisioningSettings provisioningSettings;

void onAppStateChange(callback cbFunction);

void setup() {

  initializeSerial();
  pinSetup();

  #ifdef DEBUG
    forceDelay();
  #endif

  WiFiManager::initialize();
  LedIndicators::initialize();
  DeviceControls::initialize();

  lastAppState = STARTED;
  appState = (WiFiManager::connectToWiFi() == WL_CONNECTED) ?
    CONNECT_TO_MQTT_BROKER : INITIALIZE_BLE;

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("setup() is running on core ");
  //Serial.println(coreID);

}

void loop() {

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("loop() is running on core ");
  //Serial.println(coreID);

  unsigned long currentMillis = millis();

  onEveryMS(currentMillis, 250, []{
    LedIndicators::setState(
      appState,
      WiFiManager::isConnected(),
      Sensor::isConnected(),
      payload.hasAlarm
    );
  });

  onEveryMS(currentMillis, 10000, []{
    DEBUG_PRINTF("Free heap memory: %d\n", esp_get_free_heap_size());
  });

  switch(appState) {

    case INITIALIZE_BLE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Initializing BLE services...");

        if (bleManager.initializeDeviceConfigurationService()) {
          appState = CONFIGURE_DEVICE;
        }

      });

      break;

    case CONFIGURE_DEVICE:

      onAppStateChange([]{

        WiFiManager::disconnect();
        DEBUG_PRINTLN("Waiting for WiFi configuration and device provisioning certificates...");

      });

      onEveryMS(currentMillis, 4000, []{

        String json = WiFiManager::listNetworks();
        provisioningSettings = bleManager.getDeviceConfiguration(json);

        if (provisioningSettings.isValid()) {
          appState = CONNECT_TO_WIFI;
        } else {
          DEBUG_PRINTLN("Received invalid provisioning settings; please resend.");
        }

      });

      break;

    case CONNECT_TO_WIFI:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting to WiFi...");

        if (WiFiManager::connectToWiFi(
            provisioningSettings.wiFiCredentials.ssid,
            provisioningSettings.wiFiCredentials.password) == WL_CONNECTED) {

        DEBUG_PRINTF("\nConnected to WiFi network: %s",
          provisioningSettings.wiFiCredentials.ssid.c_str());
        appState = PROVISION_DEVICE;

        } else {
          
          DEBUG_PRINTLN("Failed to connect to WiFi network.");
          appState = INITIALIZE_BLE;
          
        }

      });

      break;

    case PROVISION_DEVICE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Provisioning device...");

        Provisioning provisiong([=](bool success){
          if (success) {
            certManager.storeCertificates(provisioningSettings.certificates);
            appState = CONNECT_TO_MQTT_BROKER;
          } else {
            appState = INITIALIZE_BLE;
          }
          provisioningSettings.clear();
        });

        if (provisioningSettings.isValid()) {
          provisiong.registerDevice(provisioningSettings.certificates);
        } else {
          DEBUG_PRINTLN("Cannot provision device");
          appState = INITIALIZE_BLE; 
        }

      });

      break;

    case CONNECT_TO_MQTT_BROKER:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting device to MQTT broker...");
        
        Certificates certificates = certManager.retrieveCertificates();

        appState = (certificates.isValid() && Sensor::connect(certificates)) ?
          DEVICE_INITIALIZED : INITIALIZE_BLE;

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device initialized");

      });

      onEveryMS(currentMillis, 1000, []{

        payload = Sensor::detect();
        Sensor::report(payload);

      });

      break;

  }

}

/******************************************************************************
 * SETUP FUNCTIONS                                                            *
 *****************************************************************************/

void forceDelay() {

  unsigned short count = 0;
  unsigned const int seconds = 20;

  DEBUG_PRINTLN("Begin delay: 20 seconds");

  while(count < seconds) {
    DEBUG_PRINT(".");
    delay(1000);
    count++;
  }

  DEBUG_PRINTLN("Delay end");

}

/******************************************************************************
 * LOOP HELPERS                                                               *
 *****************************************************************************/

void onAppStateChange(callback cbFunction) {

  if (appState != lastAppState) {
    lastAppState = appState;
    cbFunction();
  }

}
