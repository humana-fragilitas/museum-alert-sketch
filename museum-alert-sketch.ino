/******************************************************************************
 * Museum Alert Arduino® Nano ESP32 Sketch                                    *
 * © Andrea Blasio, 2023-2025.                                                *
 * <| { "ssid": "Test", "password": "qyqijczyz2p37xz" } |>                    *                                      
 * humana.fragilitas@gmail.com                                                *
 * zZ&c0qIz                                                                   *
 * Client ID: MAS-EC357A188534                                                *
 * humana.fragilitas@yahoo.com                                                *
 * zZ&c0qIz                                                                   *
 * Client ID: MAS-EC357A188534                                                *
 ******************************************************************************/


#include "helpers.h"
#include "pin_setup.h"
#include "serial_com.h"
#include "config.h"
#include "ciphering.h"
#include "storage_manager.h"
#include "provisioning.h"
#include "sensor.h"
#include "wifi_manager.h"
#include "ble_manager.h"
#include "led_indicators.h"
#include "device_controls.h"
#include "json_helper.h"


AppState appState,lastAppState;
WiFiCredentialsRequest wiFiCredentialsRequest;
CertificatesRequest provisioningCertificatesRequest;
std::unique_ptr<Provisioning> provisioning;

void onAppStateChange(void (*callback)(void));


void setup() {

  SerialCom::initialize();

  #ifdef DEBUG
    DEBUG_PRINTLN("Debug mode enabled");
    forceDelay();
  #endif

  pinSetup();

  WiFiManager::initialize();
  LedIndicators::initialize();
  DeviceControls::initialize();
  Ciphering::initialize();
  Sensor::initialize();
  BLEManager::initialize();

  /**
   * Allows WiFi module to stabilise before attempting
   * any connection; removing this line causes the 
   * application to become unstable at startup
   */
  delay(2500);

  lastAppState = STARTED;

  appState = (WiFiManager::connectToWiFi() == WL_CONNECTED) ?
    CONNECT_TO_MQTT_BROKER : CONFIGURE_WIFI;

}

void loop() {

  unsigned long currentMillis = millis();

  onEveryMS(currentMillis, Timing::LED_INDICATORS_STATE_INTERVAL_MS, []{

    LedIndicators::setState(
      appState,
      WiFiManager::isConnected(),
      Sensor::isConnected(),
      Sensor::isAlarmActive()
    );

  });

  onEveryMS(currentMillis, Timing::DEVICE_CONTROLS_PROCESSOR_INTERVAL_MS,
    DeviceControls::process);

  #ifdef DEBUG
    onEveryMS(currentMillis,
      Timing::FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS,
      logHeapMemory);
  #endif

  switch(appState) {

    case CONFIGURE_WIFI: {

      onAppStateChange([]{

        WiFiManager::disconnect();
        DEBUG_PRINTLN("Waiting for WiFi credentials...");

      });

      JsonDocument doc;
      auto networkListJson = doc.to<JsonArray>();
      WiFiManager::listNetworks(networkListJson);
      SerialCom::send(USBMessageType::WIFI_NETWORKS_LIST, "", networkListJson);

      auto wiFiCredentialsJson = SerialCom::getStringWithMarkers();
      wiFiCredentialsRequest = JsonHelper::parse<WiFiCredentialsRequest>(wiFiCredentialsJson);

      if (wiFiCredentialsRequest.payload.isValid()) {

        DEBUG_PRINTLN("Received WiFi credentials");
        SerialCom::acknowledge(wiFiCredentialsRequest.correlationId);
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
          wiFiCredentialsRequest.payload.ssid.c_str(),
          wiFiCredentialsRequest.payload.password.c_str()
        ) == WL_CONNECTED) {

        DEBUG_PRINTF("Connected to WiFi network: %s\n", wiFiCredentialsRequest.payload.ssid.c_str());
        appState = CONNECT_TO_MQTT_BROKER;
        wiFiCredentialsRequest.payload.clear();

        } else {

          SerialCom::error(ErrorType::FAILED_WIFI_CONNECTION_ATTEMPT);
          DEBUG_PRINTLN("Failed to connect to WiFi network with the provided credentials; "
                        "going back to WiFi configuration mode...");
          appState = CONFIGURE_WIFI;
          
        }

      });

      break;

    case CONFIGURE_CERTIFICATES:

      onAppStateChange([]{

        DEBUG_PRINTLN("Waiting for device provisioning certificates...");

      });

      onEveryMS(currentMillis, Timing::DEVICE_CONFIGURATION_SCAN_INTERVAL_MS, []{
        
        auto provisioningCertificatesJson = SerialCom::getStringWithMarkers();

        provisioningCertificatesRequest = JsonHelper::parse<CertificatesRequest>(
          provisioningCertificatesJson
        );

        if (provisioningCertificatesRequest.payload.isValid()) {

          DEBUG_PRINTLN("Received provisioning settings:");
          DEBUG_PRINTF("- client certificate: %s\n", provisioningCertificatesRequest.payload.clientCert.c_str());
          DEBUG_PRINTF("- private key: %s\n", provisioningCertificatesRequest.payload.privateKey.c_str());
          DEBUG_PRINTF("- AWS Amplify session identity token: %s\n", provisioningCertificatesRequest.payload.idToken.c_str());

          SerialCom::acknowledge(provisioningCertificatesRequest.correlationId);
          appState = PROVISION_DEVICE;

        } else {

          DEBUG_PRINTLN("Received invalid provisioning settings; please resend... ");
          SerialCom::error(ErrorType::INVALID_DEVICE_PROVISIONING_SETTINGS);

        }

      });

      break;

    case PROVISION_DEVICE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Provisioning device...");

        provisioning.reset(new Provisioning([](bool success, DeviceConfiguration configuration) {

          if (!success || !configuration.isValid()) {

            SerialCom::error(ErrorType::FAILED_DEVICE_PROVISIONING_ATTEMPT); 
            DEBUG_PRINTLN("Cannot retrieve TLS certificate and private key; going back to configuration mode...");
            appState = CONFIGURE_CERTIFICATES;
            provisioningCertificatesRequest.payload.clear();
            provisioning.reset();
            return;

          }

          DEBUG_PRINTLN("Device successfully registered; proceeding to store device configuration: "
                        "TLS certificate, private key and associated company name...");

          if (!StorageManager::save<DeviceConfiguration>(configuration)) {

            SerialCom::error(ErrorType::FAILED_PROVISIONING_SETTINGS_STORAGE);
            DEBUG_PRINTLN("Failed to store TLS certificate, private key and associated company name; "
                          "please reset your device and repeat the provisioning procedure again");
            provisioningCertificatesRequest.payload.clear();
            provisioning.reset();
            appState = FATAL_ERROR;
            return;

          }

          appState = CONNECT_TO_MQTT_BROKER;
          provisioningCertificatesRequest.payload.clear();
          provisioning.reset();

        }));

        if (!provisioningCertificatesRequest.payload.isValid()) {

          SerialCom::error(ErrorType::INVALID_DEVICE_PROVISIONING_SETTINGS);
          DEBUG_PRINTLN("Cannot provision device: received invalid provisioning certificates");
          appState = FATAL_ERROR;
          return;

        }

        provisioning->registerDevice(provisioningCertificatesRequest.payload);

      });

      break;

    case CONNECT_TO_MQTT_BROKER:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting device to MQTT broker...");

        auto configuration = StorageManager::load<DeviceConfiguration>();

        if (!configuration.isValid()) {

          if (lastAppState == STARTED) {
            DEBUG_PRINTLN("Device configuration retrieval failed: possible corrupted storage");
            SerialCom::error(ErrorType::FAILED_DEVICE_CONFIGURATION_RETRIEVAL);
            appState = FATAL_ERROR;
          } else {
            DEBUG_PRINTLN("Device configuration retrieval failed: device is yet to be registered");
            appState = CONFIGURE_CERTIFICATES;
          }

          return;

        }

        Sensor::configure(configuration);
        
        if (WiFiManager::isConnected() && Sensor::connect()) {

          appState = DEVICE_INITIALIZED;
          
        } else {

          DEBUG_PRINTLN("Could not connect to MQTT broker");
          SerialCom::error(ErrorType::FAILED_MQTT_BROKER_CONNECTION);
          appState = FATAL_ERROR;

        }

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device initialized");

        // TO DO: add this to alarm distance as one entry: configuration
        StorageManager::save<BeaconURL>("https://google.com");

        Sensor::setDistance(
          StorageManager::load<Distance>()
        );
        Sensor::setBroadcastUrl(
          StorageManager::load<BeaconURL>()
        );

      });

      onEveryMS(currentMillis, Timing::SENSOR_DETECTION_INTERVAL_MS, []{

        if(!Sensor::detect()) {

          SerialCom::error(ErrorType::FAILED_SENSOR_DETECTION_REPORT);
          DEBUG_PRINTF("Sensor cannot send detection payload... Detecting again in %d seconds\n",
              Timing::SENSOR_DETECTION_INTERVAL_MS / 1000);

        }

      });

      onEveryMS(currentMillis, Timing::BEACON_MAINTENANCE_INTERVAL_MS,
          BLEManager::maintainBeacon);

    break;

    case FATAL_ERROR:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device is in error state and needs to be reset");

      });

      onEveryMS(currentMillis, Timing::USB_COMMANDS_SCAN_INTERVAL_MS, []{

        auto usbCommandJson = SerialCom::getStringWithMarkers();
        auto command = JsonHelper::parse<DeviceCommandRequest>(usbCommandJson);

        if (command.payload == USBCommandType::USB_COMMAND_INVALID) {
          DEBUG_PRINTLN("Device received an invalid command via USB");
          SerialCom::error(ErrorType::INVALID_DEVICE_COMMAND);
          return;
        }

        SerialCom::acknowledge(command.correlationId);
        
        switch (command.payload) {
          case USBCommandType::HARD_RESET:
             DeviceControls::reset();
             break;
          // further commands here...
          default:
            DEBUG_PRINTF("Device received an unhandled command "
                         "via USB with id: %d\n", command);
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
    SerialCom::send(USBMessageType::APP_STATE, "", appStateJson);
    cbFunction();
    
  }

}

/******************************************************************************
 * DEBUGGING HELPER FUNCTIONS                                                 *
 ******************************************************************************/

void logHeapMemory(void){

  DEBUG_PRINTLN("--- Heap memory consumption -----------------------------");
  size_t freeHeap = ESP.getFreeHeap();
  size_t minFreeHeap = ESP.getMinFreeHeap();
  DEBUG_PRINTF("Heap: %d free, %d min\n", freeHeap, minFreeHeap);
  DEBUG_PRINTLN("---------------------------------------------------------");

}