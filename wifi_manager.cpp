#include "wifi_manager.h"

void WiFiManager::initialize() {
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiManager::onWiFiEvent);
    
    // DON'T start monitoring immediately - wait for system to stabilize
    DEBUG_PRINTLN("WiFi Manager initialized - monitoring will start after delay");
}

void WiFiManager::startMonitoring() {
    if (wifiMonitorTaskHandle != nullptr) {
        DEBUG_PRINTLN("WiFi monitoring already running");
        return;
    }
    
    // Add startup delay to ensure WiFi subsystem is ready
    DEBUG_PRINTLN("Starting WiFi monitoring with initialization delay...");
    delay(2000);  // Wait 2 seconds for system to stabilize
    
    monitoringEnabled = true;
    wasConnected = WiFi.isConnected();
    lastConnectivityTest = 0;
    
    xTaskCreate(
        wifiMonitorTaskWrapper,
        "WiFi_Monitor_Task",
        8192,  // Increased stack size
        nullptr,
        1,     // Lower priority than MQTT task
        &wifiMonitorTaskHandle
    );
    
    DEBUG_PRINTLN("WiFi monitoring task started");
}

void WiFiManager::wifiMonitorTask() {
    DEBUG_PRINTLN("WiFi Monitor: Task started");
    
    while (monitoringEnabled) {
        // Check WiFi status
        bool currentlyConnected = WiFi.isConnected();
        wl_status_t wifiStatus = WiFi.status();
        
        // Detect state changes that events might have missed
        if (wasConnected && !currentlyConnected) {
            DEBUG_PRINTLN("WiFi Monitor: Disconnection detected (event may have been missed)");
            handleConnectionStateChange(false);
        } else if (!wasConnected && currentlyConnected) {
            DEBUG_PRINTLN("WiFi Monitor: Connection detected");
            handleConnectionStateChange(true);
        }
        
        // If we think we're connected, test actual connectivity every 30 seconds
        if (currentlyConnected && (millis() - lastConnectivityTest > 30000)) {
            lastConnectivityTest = millis();
            
            if (!testConnectivity()) {
                DEBUG_PRINTLN("WiFi Monitor: Connected but no actual internet connectivity");
                
                // Force reconnection
                WiFi.disconnect();
                vTaskDelay(pdMS_TO_TICKS(1000));
                WiFi.reconnect();
                
                // Wait for reconnection attempt
                int attempts = 0;
                while (WiFi.status() != WL_CONNECTED && attempts < 20 && monitoringEnabled) {
                    vTaskDelay(pdMS_TO_TICKS(500));
                    attempts++;
                }
                
                if (WiFi.status() == WL_CONNECTED) {
                    DEBUG_PRINTLN("WiFi Monitor: Reconnection successful");
                } else {
                    DEBUG_PRINTLN("WiFi Monitor: Reconnection failed");
                }
            }
        }
        
        // If disconnected, attempt reconnection
        if (!currentlyConnected && wifiStatus != WL_CONNECTED) {
            DEBUG_PRINTF("WiFi Monitor: WiFi status %d, attempting reconnection\n", wifiStatus);
            
            // Clean disconnect first
            if (wifiStatus != WL_DISCONNECTED) {
                WiFi.disconnect();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            
            // Attempt reconnection
            WiFi.reconnect();
            
            // Wait for reconnection with timeout
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 20 && monitoringEnabled) {
                vTaskDelay(pdMS_TO_TICKS(500));
                attempts++;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                DEBUG_PRINTLN("WiFi Monitor: Automatic reconnection successful");
            } else {
                DEBUG_PRINTLN("WiFi Monitor: Automatic reconnection failed, will retry");
            }
        }
        
        wasConnected = currentlyConnected;
        
        // Check every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
    DEBUG_PRINTLN("WiFi Monitor: Task exiting");
    wifiMonitorTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

bool WiFiManager::testConnectivity() {
    // Test actual internet connectivity
    WiFiClient testClient;
    testClient.setTimeout(5000); // 5 second timeout
    
    // Try to connect to Google DNS
    bool canConnect = testClient.connect("8.8.8.8", 53);
    if (canConnect) {
        testClient.stop();
        DEBUG_PRINTLN("WiFi Monitor: Connectivity test passed");
        return true;
    }
    
    DEBUG_PRINTLN("WiFi Monitor: Connectivity test failed");
    return false;
}

void WiFiManager::handleConnectionStateChange(bool connected) {
    if (connected) {
        DEBUG_PRINTLN("WiFi Monitor: Connection state changed to CONNECTED");
        // Reset connectivity test timer on new connection
        lastConnectivityTest = millis();
    } else {
        DEBUG_PRINTLN("WiFi Monitor: Connection state changed to DISCONNECTED");
    }
}

bool WiFiManager::isMonitoringActive() {
    return (wifiMonitorTaskHandle != nullptr && monitoringEnabled);
}

void WiFiManager::wifiMonitorTaskWrapper(void* pvParameters) {
    // Since it's a static class, we can call the static method directly
    wifiMonitorTask();
}

void WiFiManager::listNetworks(JsonArray& arr) {
  
  byte numSsid = WiFi.scanNetworks();
  DEBUG_PRINTF("Number of available WiFi networks: %d\n", numSsid);

  // Define a struct to hold WiFi info
  struct WiFiEntry {
    String ssid;
    int rssi;
    int encryptionType;
  };

  std::vector<WiFiEntry> networks;

  for (int i = 0; i < numSsid; ++i) {
    networks.push_back({ WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i) });
  }

  WiFi.scanDelete();

  std::sort(networks.begin(), networks.end(), [](const WiFiEntry &a, const WiFiEntry &b) {
    return a.rssi > b.rssi;
  });

  //int maxResults = std::min(10, static_cast<int>(networks.size()));

  // JsonDocument doc;
  //JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < networks.size(); ++i) {
    JsonObject wiFiEntry = arr.add<JsonObject>();
    wiFiEntry["ssid"] = networks[i].ssid;
    wiFiEntry["rssi"] = networks[i].rssi;
    wiFiEntry["encryptionType"] = networks[i].encryptionType;

    DEBUG_PRINTF("%d) %s | signal: %d dBm | encryption: %d\n",
                  i + 1, networks[i].ssid.c_str(), networks[i].rssi, networks[i].encryptionType);
  }

  String jsonString;
  serializeJson(arr, jsonString);

  DEBUG_PRINTLN(jsonString.c_str());

  //return arr;

}

uint8_t WiFiManager::connectToWiFi(const char *ssid, const char *pass) {

  if (!ssid || ssid[0] == '\0' || !pass) {
      DEBUG_PRINTLN("Invalid WiFi credentials");
      return WL_CONNECT_FAILED;
  }

  DEBUG_PRINTF("Connecting to WiFi SSID: %s\n", ssid);
  DEBUG_PRINTF("Connecting to WiFi password: %s\n", pass);

  WiFi.disconnect(true);
  // delay(250); TO DO: is it necessary?
  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult(10000);

}

uint8_t WiFiManager::connectToWiFi() {

  /*
    Serial.printf("SSID: %s, PASSWORD: %s\n", WiFi.SSID().c_str(), WiFi.psk().c_str());
  */

  DEBUG_PRINTF("Trying to connect to a previously set WiFi endpoint; waiting for %d seconds...\n", (Timing::WIFI_AUTO_CONNECTION_TIMEOUT_MS / 1000));
  WiFi.begin();
  return WiFi.waitForConnectResult(Timing::WIFI_AUTO_CONNECTION_TIMEOUT_MS);

}

bool WiFiManager::eraseConfiguration() {

  bool success = WiFi.eraseAP();
  esp_wifi_start();

  DEBUG_PRINTLN(success ? "Erased WiFi configuration" : "Failed to erase WiFi configuration");

  return success;

}

bool WiFiManager::disconnect(bool wiFiOff, bool eraseAp) {

  bool success = WiFi.disconnect(wiFiOff, eraseAp);

  DEBUG_PRINTLN(success ? "Disconnected from WiFi network" : "Failed to disconnect from WiFi network");

  return success;

}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {
    static unsigned long lastEventTime = 0;
    unsigned long currentTime = millis();
    
    // Prevent event spam
    if (currentTime - lastEventTime < 1000 && 
        (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED || 
         event == ARDUINO_EVENT_WIFI_STA_CONNECTED)) {
        return;
    }
    lastEventTime = currentTime;

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            DEBUG_PRINTLN("WiFi Event: Interface ready");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            DEBUG_PRINTLN("WiFi Event: Client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            DEBUG_PRINTLN("WiFi Event: Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            DEBUG_PRINTLN("WiFi Event: Disconnected from access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            DEBUG_PRINTF("WiFi Event: Got IP address: %s\n", WiFi.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            DEBUG_PRINTLN("WiFi Event: Lost IP address");
            break;
        default:
            DEBUG_PRINTF("WiFi Event: %d\n", event);
            break;
    }
}

bool WiFiManager::isConnected() {
  return WiFi.isConnected();
}

// possible fix to alternate wifi and ble
void WiFiManager::reset() {

  esp_wifi_stop();  // Stop WiFi driver
  esp_wifi_deinit(); // Deinitialize WiFi
  delay(100);       // Short delay
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);  // Reinitialize WiFi
  esp_wifi_start(); // Start WiFi again

}

TaskHandle_t WiFiManager::wifiMonitorTaskHandle = nullptr;
bool WiFiManager::monitoringEnabled = false;
bool WiFiManager::wasConnected = false;
unsigned long WiFiManager::lastConnectivityTest = 0;