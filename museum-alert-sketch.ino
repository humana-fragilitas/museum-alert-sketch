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

// Client ID: MAS-EC357A188534

// Mutex mutex;
TaskHandle_t ledIndicatorsTask;

AppState appState,lastAppState;

BLEManager bleManager;
CertManager certManager;
ProvisioningSettings provisioningSettings;

unsigned const int configureWiFiInterval = 4000;
unsigned const int sensorInterval = 1000;
unsigned const int resetButtonInterval = 4000;
unsigned int previousResetButtonInterval = 0;

void setup() {

  initializeSerial();

  pinSetup();

  #ifdef DEBUG
    forceDelay();
  #endif

  initializeUI();

  WiFiManager::initialize();

  // DeviceControls::initialize(); /* interrupt handling for reset button here */

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
    LedIndicators::setState(appState, WiFiManager::isConnected(), Sensor::isConnected());
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

      onEveryMS(currentMillis, configureWiFiInterval, []{

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

      onEveryMS(currentMillis, sensorInterval, []{

        AlarmPayload payload = Sensor::detect();
        digitalWrite(Pins::Alarm, payload.hasAlarm);
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

void initializeUI() {

  attachInterrupt(digitalPinToInterrupt(Pins::ResetButton), onResetButtonISR, CHANGE);

  xTaskCreatePinnedToCore(
    ledIndicators,
    "LedIndicators",
    1024,
    NULL,
    0,
    &ledIndicatorsTask,
    0
  );

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

/******************************************************************************
 * CALLBACK FUNCTIONS                                                         *
 *****************************************************************************/

void onResetButtonISR(void) {

  // Note: reset button current is pulled down via 10K OHM resistance

  unsigned long currentMillis = millis();

  if (digitalRead(Pins::ResetButton)) {

    previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - previousResetButtonInterval >= resetButtonInterval) {

      DEBUG_PRINTLN("Reset button pressed...");
      DEBUG_PRINTLN("Erasing AP settings and rebooting...");
      WiFi.eraseAP();
      ESP.restart();
      //wiFiManager.disconnect(true, false);
      //WiFi.eraseAP();
      // esp_wifi_start();
      // Note: first restart after serial flashing causes puts the board in boot mode:(1,7) (purple led)
      // https://github.com/esp8266/Arduino/issues/1722
      // https://github.com/esp8266/Arduino/issues/1017
      // https://github.com/esp8266/Arduino/issues/1722#issuecomment-321818357


    }

  }

}

/******************************************************************************
 * MULTITHREADING FUNCTIONS                                                   *
 ******************************************************************************/

void ledIndicators(void *parameter) {

  const unsigned long slowInterval = 500;
  const unsigned long mediumInterval = 250;
  const unsigned long fastInterval = 125;

  for(;;) {

    unsigned long currentMillis = millis();

    switch(appState) {

      case CONFIGURE_DEVICE:
        onEveryMS(currentMillis, mediumInterval, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });
        break;
      default:
        onEveryMS(currentMillis, slowInterval, []{
          digitalWrite(Pins::Status, !digitalRead(Pins::Status));
        });

    }
    
  }

}

