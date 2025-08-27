#include "ble_manager.h"
#include <utility>

bool BLEManager::initialized = false;
bool BLEManager::beaconActive = false;
String BLEManager::currentUrl = "";
BLEAdvertising* BLEManager::pAdvertising = nullptr;
unsigned long BLEManager::lastMaintenanceTime = 0;

void BLEManager::initialize() noexcept {

   if (initialized) return;

   BLEDevice::init(std::string(Sensor::name));
   pAdvertising = BLEDevice::getAdvertising();

   // Set power level for advertising
   esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);

   DEBUG_PRINTF("BLE interface initialized with device name: %s\n", Sensor::name);

   initialized = true;

}

void BLEManager::startBeacon(const String& url) noexcept {

   if (url.isEmpty()) {
     DEBUG_PRINTLN("BLE Manager received an empty URL; "
            "skipping beacon initialization...");
     return;
   }

   if (!initialized) {
     DEBUG_PRINTLN("BLE Manager not initialized");
     return;
   }

   if (beaconActive && currentUrl == url) {
     DEBUG_PRINTLN("Beacon already active with same URL");
     return;
   }

   DEBUG_PRINTF("Starting BLE beacon with URL: %s\n", url.c_str());

   if (beaconActive) {
     stopBeacon();
   }

   currentUrl = url;

   // Create Eddystone-URL beacon payload
   const auto encodedUrl = encodeUrl(url);
   std::string serviceData;

   constexpr size_t maxEncodedUrlLength = 18;
   if (encodedUrl.length() > maxEncodedUrlLength) {

     DEBUG_PRINTF("Eddystone beacon URL too long after encoding: %d bytes (max %zu)\n", 
           encodedUrl.length(), maxEncodedUrlLength);
     DEBUG_PRINTF("Encoded URL: %s\n", encodedUrl.c_str());
     DEBUG_PRINTLN("Cannot start Eddystone beacon broadcasting");
     return;

   }

   constexpr uint8_t eddystoneUrlFrameType = 0x10;
   constexpr int8_t txPowerLevel = -6;
   
   serviceData += static_cast<char>(eddystoneUrlFrameType); // Frame Type: URL
   /**
    * Distance calibration has been established empirically:
    * the TX power value has been determined through real-world testing.
    * Current value: -6 dBm provides accurate distance at 30cm.
    * 
    * If distance estimation is inaccurate:
    * - test device at exactly 30cm from your beacon detection app;
    * - increase value (+3, +4) if distance shows too far;
    * - decrease value (+1, 0, -1) if distance shows too close;
    * - update this comment with new findings.
    */
   serviceData += static_cast<char>(txPowerLevel);

   for (const auto& ch : encodedUrl) {
     serviceData += ch;
   }

   BLEAdvertisementData advData;
   advData.setFlags(0x06); // BR_EDR_NOT_SUPPORTED | GENERAL_DISC_MODE

   advData.setServiceData(BLEUUID((uint16_t)0xFEAA), serviceData);

   pAdvertising->setAdvertisementData(advData);
   pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
   pAdvertising->setMinInterval(Timing::BEACON_MIN_INTERVAL_UNITS); // 160 × 0.625ms = 100ms 
   pAdvertising->setMaxInterval(Timing::BEACON_MAX_INTERVAL_UNITS); // 320 × 0.625ms = 200ms

   pAdvertising->start();

   beaconActive = true;
   lastMaintenanceTime = millis();

   DEBUG_PRINTLN("BLE beacon started successfully");

}


void BLEManager::stopBeacon() noexcept {

   if (pAdvertising) pAdvertising->stop();
   beaconActive = false;
   currentUrl = "";

}

void BLEManager::maintainBeacon() noexcept {

   if (!initialized || !beaconActive) return;
   
   DEBUG_PRINTLN("Performing BLE beacon maintenance");

   if (pAdvertising) {
     pAdvertising->stop();
     delay(100);
     pAdvertising->start();
   }

}

void BLEManager::cleanup() noexcept {

   if (beaconActive) stopBeacon();
   if (initialized) {
     BLEDevice::deinit(false);
     initialized = false;
   }

}

bool BLEManager::isBeaconActive() noexcept {
   return beaconActive;
}

String BLEManager::encodeUrl(const String& url) {
   DEBUG_PRINTF("=== URL ENCODING DEBUG ===\n");
   DEBUG_PRINTF("Original URL: %s (length: %d)\n", url.c_str(), url.length());
   
   String encoded{""};
   String workingUrl{url};

   const auto schemePrefix = getUrlSchemePrefix(workingUrl);
   encoded += static_cast<char>(schemePrefix);
   DEBUG_PRINTF("Scheme prefix: 0x%02X\n", schemePrefix);

   // Remove scheme
   if (workingUrl.startsWith("http://www.")) workingUrl = workingUrl.substring(11);
   else if (workingUrl.startsWith("https://www.")) workingUrl = workingUrl.substring(12);
   else if (workingUrl.startsWith("http://")) workingUrl = workingUrl.substring(7);
   else if (workingUrl.startsWith("https://")) workingUrl = workingUrl.substring(8);
   
   DEBUG_PRINTF("After scheme removal: %s (length: %d)\n", workingUrl.c_str(), workingUrl.length());

   workingUrl = compressUrl(workingUrl);
   DEBUG_PRINTF("After compression: ");
   
   constexpr char minPrintableChar = 32;
   constexpr char maxPrintableChar = 126;
   
   for (const auto& ch : workingUrl) {
     if (ch >= minPrintableChar && ch <= maxPrintableChar) {
      DEBUG_PRINTF("%c", ch);
     } else {
      DEBUG_PRINTF("[0x%02X]", static_cast<unsigned char>(ch));
     }
   }
   DEBUG_PRINTF(" (length: %d)\n", workingUrl.length());
   
   encoded += workingUrl;

   DEBUG_PRINTF("Final encoded length: %d bytes\n", encoded.length());
   DEBUG_PRINTF("Final encoded (hex): ");
   for (const auto& ch : encoded) {
     DEBUG_PRINTF("%02X ", static_cast<unsigned char>(ch));
   }
   DEBUG_PRINTF("\n=========================\n");

   return encoded;
}

uint8_t BLEManager::getUrlSchemePrefix(const String& url) noexcept {

   if (url.startsWith("http://www.")) return 0x00;
   if (url.startsWith("https://www.")) return 0x01;
   if (url.startsWith("http://")) return 0x02;
   if (url.startsWith("https://")) return 0x03;
   return 0x03;  // Default to https://

}

String BLEManager::compressUrl(const String& url) {

   DEBUG_PRINTF("Before compression: %s\n", url.c_str());
   
   String compressed{url};
   
   // Test each replacement
   if (compressed.indexOf(".sh/") >= 0) {
     DEBUG_PRINTF("Found .sh/ - compressing\n");
     compressed.replace(".sh/", "\x0E");
   }
   if (compressed.indexOf(".sh") >= 0) {
     DEBUG_PRINTF("Found .sh - compressing\n");
     compressed.replace(".sh", "\x0F");
   }

   compressed.replace(".com/", "\x00");
   compressed.replace(".org/", "\x01");
   compressed.replace(".edu/", "\x02");
   compressed.replace(".net/", "\x03");
   compressed.replace(".info/", "\x04");
   compressed.replace(".biz/", "\x05");
   compressed.replace(".gov/", "\x06");
   compressed.replace(".com", "\x07");
   compressed.replace(".org", "\x08");
   compressed.replace(".edu", "\x09");
   compressed.replace(".net", "\x0A");
   compressed.replace(".info", "\x0B");
   compressed.replace(".biz", "\x0C");
   compressed.replace(".gov", "\x0D");
   compressed.replace(".ly/", "\x10");   // bit.ly, dub.ly
   compressed.replace(".ly", "\x11");
   compressed.replace(".co/", "\x12");   // t.co, etc.
   compressed.replace(".co", "\x13");
   compressed.replace(".sh/", "\x0E");  // Use next available code
   compressed.replace(".sh", "\x0F");   // Use next available code

   DEBUG_PRINTF("After compression: %s\n", compressed.c_str());

   return compressed;

}
