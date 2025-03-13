/******************************************************************************
 * Museum Alert Arduino® Nano ESP32 Sketch                                    *
 * © Andrea Blasio, 2023-2025.                                                *
 * <| { "ssid": "Test", "password": "qyqijczyz2p37xz" } |>                    *                                      
 * humana.fragilitas@gmail.com                                                *
 * zZ&c0qIz                                                                   *
 ******************************************************************************/
#include <memory>

#include <esp_heap_caps.h>

// erase non-volatile storage
#include "nvs_flash.h"

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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Client ID: MAS-EC357A188534

AppState appState,lastAppState;

BLEManager bleManager;
std::unique_ptr<Provisioning> provisioning;
WiFiCredentials wiFiCredentials;
Certificates provisioningCertificates;

void onAppStateChange(void (*callback)(void));

void setup() {

  #ifdef DEBUG
    SerialCom::initialize();
    DEBUG_PRINTLN("Debug mode enabled");
    forceDelay();
  #endif

  // TO DO: remove after testing and move in a dedicated reset method
  // erase non-volatile storage; also deletes wifi configuration!
  nvs_flash_erase();
  nvs_flash_init();

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

  onEveryMS(currentMillis, Timing::LED_INDICATORS_STATE_INTERVAL_MS, []{

    LedIndicators::setState(
      appState,
      WiFiManager::isConnected(),
      Sensor::isConnected(),
      Sensor::hasAlarm()
    );
    
  });

  #ifdef DEBUG
    onEveryMS(currentMillis, Timing::FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS, []{
      DEBUG_PRINTLN("--- Memory consumption ----------------------------------");
      DEBUG_PRINTLN("[ Heap Info ]");
      DEBUG_PRINTF("Free heap: %d bytes\n", esp_get_free_heap_size());
      DEBUG_PRINTF("Largest free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      DEBUG_PRINTF("Minimum ever free heap: %d bytes\n", esp_get_minimum_free_heap_size());
      DEBUG_PRINTLN("---------------------------------------------------------");
    });
  #endif

  switch(appState) {

    case INITIALIZE_CIPHERING:

      onAppStateChange([]{

        DEBUG_PRINTLN("Initializing ciphering...");

        if (Ciphering::initialize()) {
          appState = CONFIGURE_WIFI;
        } else {
          SerialCom::error(ErrorType::CIPHERING_INITIALIZATION_ERROR);
          DEBUG_PRINTLN("Could not initialize ciphering; please reset");
        }

      });
      
      break;

    case CONFIGURE_WIFI: {

      onAppStateChange([]{

        WiFiManager::disconnect();
        SerialCom::initialize();
        DEBUG_PRINTLN("Waiting for WiFi credentials...");

      });

      JsonDocument doc;
      JsonArray networkListJson = doc.to<JsonArray>();
      WiFiManager::listNetworks(networkListJson);
      
      SerialCom::send(MessageType::WIFI_NETWORKS_LIST, networkListJson);

      String wiFiCredentialsJson = SerialCom::getStringWithMarkers();
      wiFiCredentials = Provisioning::parseWiFiCredentialsJSON(wiFiCredentialsJson);

      if (wiFiCredentials.isValid()) {
        DEBUG_PRINTLN("Received valid WiFi credentials");
        appState = CONNECT_TO_WIFI;
      } else {
        SerialCom::error(ErrorType::INVALID_WIFI_CREDENTIALS);
        DEBUG_PRINTLN("Received invalid WiFi credentials");
      }

    }

      break;

    case CONNECT_TO_WIFI:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting to WiFi...");

        if (WiFiManager::connectToWiFi(
          wiFiCredentials.ssid.c_str(),
          wiFiCredentials.password.c_str()
        ) == WL_CONNECTED) {

          DEBUG_PRINTF("Connected to WiFi network: %s\n", wiFiCredentials.ssid.c_str());
          appState = CONFIGURE_CERTIFICATES;

          wiFiCredentials.clear();

        } else {
          SerialCom::error(ErrorType::FAILED_WIFI_CONNECTION_ATTEMPT);
          DEBUG_PRINTLN("Failed to connect to WiFi network with the provided credentials... Going back to WiFi configuration mode");
          delay(2000); // TO DO: what if it is removed? Is it still necessary?
          appState = CONFIGURE_WIFI;
          
        }

      });

      break;

    case CONFIGURE_CERTIFICATES:

      onAppStateChange([]{

        SerialCom::initialize();
        DEBUG_PRINTLN("Waiting for device provisioning certificates...");

      });

      onEveryMS(currentMillis, Timing::DEVICE_CONFIGURATION_SCAN_INTERVAL_MS, []{

        String provisioningCertificatesJson = SerialCom::getStringWithMarkers();

        provisioningCertificates = Provisioning::parseProvisioningCertificates(provisioningCertificatesJson);
        bool validSettings; 

        DEBUG_PRINTF("CERTS: %s, %s, %s", provisioningCertificates.clientCert.c_str(),
            provisioningCertificates.privateKey.c_str(),
            provisioningCertificates.idToken.c_str());

        if ((validSettings = provisioningCertificates.isValid())) {
          appState = PROVISION_DEVICE;
        } else {
          SerialCom::error(ErrorType::INVALID_DEVICE_PROVISIONING_SETTINGS);
        }

        DEBUG_PRINTF("Received %s provisioning settings\n",
            (validSettings ? "valid" : "invalid"));

      });

      break;

    case PROVISION_DEVICE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Provisioning device...");

        provisioning.reset(new Provisioning([](bool success, DeviceConfiguration configuration) {

            if (success && configuration.isValid()) {
                DEBUG_PRINTLN("Device successfully registered; proceeding to store TLS certificate and private key...");
                if (CertManager::store(configuration)) {
                    appState = CONNECT_TO_MQTT_BROKER;
                } else {
                    SerialCom::error(ErrorType::FAILED_PROVISIONING_SETTINGS_STORAGE);
                    DEBUG_PRINTLN("Failed to store TLS certificate and private key; please reset your device and repeat the provisioning procedure again.");
                }
            } else {
                SerialCom::error(ErrorType::FAILED_DEVICE_PROVISIONING_ATTEMPT); // TO DO: refine this for at least two cases: 1) generic error; 2) device already exists
                DEBUG_PRINTLN("Cannot retrieve TLS certificate and private key; going back to TLS configuration...");
                appState = CONFIGURE_CERTIFICATES;
            }

            provisioningCertificates.clear();
            //TO DO: this is new; test it; may disrupt application
            provisioning.reset();

        }));

        if (provisioningCertificates.isValid()) {
          provisioning->registerDevice(provisioningCertificates);
        } else {
          SerialCom::error(ErrorType::INVALID_DEVICE_PROVISIONING_SETTINGS);
          DEBUG_PRINTLN("Cannot provision device: received invalid provisioning certificates; going back to configuration step");
          appState = CONFIGURE_CERTIFICATES;
        }

      });

      break;

    case CONNECT_TO_MQTT_BROKER:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting device to MQTT broker...");
          
        DeviceConfiguration configuration = CertManager::retrieve();
        Sensor::configure(configuration);

        DEBUG_PRINTF("PEM Cert: %s\n", configuration.certificates.clientCert.c_str());
        DEBUG_PRINTF("Private key: %s\n", configuration.certificates.privateKey.c_str());
        
        if(configuration.isValid() && Sensor::connect()) {

          appState = DEVICE_INITIALIZED;

        } else {

          SerialCom::error(ErrorType::FAILED_MQTT_BROKER_CONNECTION);
          appState = CONFIGURE_CERTIFICATES;

        }

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device initialized");

      });

      onEveryMS(currentMillis, Timing::SENSOR_DETECTION_INTERVAL_MS, []{

        if(!Sensor::detect()) {

          SerialCom::error(ErrorType::FAILED_SENSOR_DETECTION_REPORT);
          DEBUG_PRINTF("Sensor cannot send detection payload... Retrying in %d seconds\n",
              Timing::SENSOR_DETECTION_INTERVAL_MS / 1000);

        }

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
  unsigned const int milliseconds = Timing::DEBUG_FORCED_INITIALIZATION_DELAY_MS;

  DEBUG_PRINTF("Begin delay: %d seconds\n", (Timing::DEBUG_FORCED_INITIALIZATION_DELAY_MS / 1000));

  while(count < milliseconds) {
    DEBUG_PRINT(".");
    delay(interval);
    count += interval;
  }

  DEBUG_PRINTLN("\nDelay end");

}

/******************************************************************************
 * FINITE STATE MACHINE HELPER FUNCTIONS                                      *
 ******************************************************************************/

void onAppStateChange(void (*cbFunction)(void)) {

  if (appState != lastAppState) {

    JsonDocument appStateJson;
    appStateJson["appState"] = appState;

    lastAppState = appState;
    SerialCom::send(MessageType::APP_STATE, appStateJson);
    cbFunction();
    
  }

}