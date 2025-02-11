/******************************************************************************
 * Museum Alert Arduino® Nano ESP32 Sketch                                    *
 * © Andrea Blasio, 2023-2025.                                                *
 ******************************************************************************/

#include <esp_heap_caps.h>

#include "macros.h"
#include "helpers.h"
#include "pins.h"
#include "pin_setup.h"
#include "serial_com.h"
#include "settings.h"
#include "ciphering.h"
#include "cert_manager.h"
#include "provisioning.h"
#include "sensor.h"
#include "wifi_manager.h"
#include "ble_manager.h"
#include "led_indicators.h"
#include "device_controls.h"

// Client ID: MAS-EC357A188534

AppState appState,lastAppState;
AlarmPayload detectionPayload;

BLEManager bleManager;
ProvisioningSettings provisioningSettings;

void onAppStateChange(void (*callback)(void));

void setup() {

  #ifdef DEBUG
    initializeSerial();
    DEBUG_PRINTLN("Debug mode enabled");
    forceDelay();
  #endif

  pinSetup();

  WiFiManager::initialize();
  LedIndicators::initialize();
  DeviceControls::initialize();
  Sensor::initialize();

  lastAppState = STARTED;

  appState = (WiFiManager::connectToWiFi() == WL_CONNECTED) ?
    CONNECT_TO_MQTT_BROKER : INITIALIZE_CIPHERING;

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("setup() is running on core ");
  //Serial.println(coreID);

}

void loop() {

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("loop() is running on core ");
  //Serial.println(coreID);

  unsigned long currentMillis = millis();

  onEveryMS(currentMillis, Timing::LED_INDICATORS_STATE_MS, []{

    LedIndicators::setState(
      appState,
      WiFiManager::isConnected(),
      Sensor::isConnected(),
      detectionPayload.hasAlarm
    );
    
  });

  #ifdef DEBUG
    onEveryMS(currentMillis, Timing::FREE_HEAP_MEMORY_DEBUG_LOG_MS, []{
      DEBUG_PRINTF("Free heap memory: %d\n", esp_get_free_heap_size());
    });
  #endif

  switch(appState) {

    case INITIALIZE_CIPHERING:

      onAppStateChange([]{

        DEBUG_PRINTLN("Initializing ciphering...");

        if (Ciphering::initialize()) {
          appState = INITIALIZE_BLE;
        }

      });
      
      break;

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

      onEveryMS(currentMillis, Timing::WIFI_NETWORKS_SCAN_MS, []{

        char jsonBuffer[4096];

        WiFiManager::listNetworks(jsonBuffer);

        provisioningSettings = bleManager.getDeviceConfiguration(jsonBuffer);

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

        DEBUG_PRINTF("\nConnected to WiFi network: %s", provisioningSettings.wiFiCredentials.ssid);
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
            CertManager::storeCertificates(provisioningSettings.certificates);
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
        
        Certificates certificates = CertManager::retrieveCertificates();

        appState = (certificates.isValid() && Sensor::connect(certificates)) ?
          DEVICE_INITIALIZED : INITIALIZE_BLE;

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device initialized");

      });

      onEveryMS(currentMillis, Timing::SENSOR_DETECTION_MS, []{

        detectionPayload = Sensor::detect();
        Sensor::report(detectionPayload);

      });

      break;

  }

}

/******************************************************************************
 * SETUP HELPER                                                               *
 ******************************************************************************/

void forceDelay() {

  unsigned short count = 0;
  unsigned const short interval = 1000;
  unsigned const int milliseconds = Timing::DEBUG_FORCED_INITIALIZATION_DELAY;

  DEBUG_PRINTLN("Begin delay: 20 seconds");

  while(count < milliseconds) {
    DEBUG_PRINT(".");
    delay(interval);
    count += interval;
  }

  DEBUG_PRINTLN("Delay end");

}

/******************************************************************************
 * LOOP HELPER                                                                *
 ******************************************************************************/

void onAppStateChange(void (*cbFunction)(void)) {

  if (appState != lastAppState) {
    lastAppState = appState;
    cbFunction();
  }

}