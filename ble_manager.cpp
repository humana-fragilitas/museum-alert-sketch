#include "ble_manager.h"

bool BLEManager::initialized = false;
bool BLEManager::beaconActive = false;
String BLEManager::currentUrl = "";
BLEAdvertising* BLEManager::pAdvertising = nullptr;
unsigned long BLEManager::lastMaintenanceTime = 0;

void BLEManager::initialize() {
    if (initialized) return;

    BLEDevice::init("MuseumAlert");
    pAdvertising = BLEDevice::getAdvertising();

    // Set power level for advertising
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);

    initialized = true;
}

void BLEManager::startBeacon(const String& url) {
    if (!initialized) {
        DEBUG_PRINTLN("BLE Manager not initialized");
        return;
    }

    if (beaconActive && currentUrl == url) {
        DEBUG_PRINTLN("Beacon already active with same URL");
        return;
    }

    DEBUG_PRINTF("Starting BLE beacon with URL: %s\n", url.c_str());

    // Stop any existing beacon
    if (beaconActive) {
        stopBeacon();
    }

    currentUrl = url;

    // Create Eddystone-URL beacon payload
    String encodedUrl = encodeUrl(url); // Your encodeUrl() already returns scheme + compressed
    std::string serviceData;

    serviceData += (char)0x10;     // Frame Type: URL
    serviceData += (char)0x00;     // TX Power: 0 dBm (or adjust as needed)

    for (size_t i = 0; i < encodedUrl.length(); i++) {
        serviceData += encodedUrl[i];
    }

    BLEAdvertisementData advData;
    advData.setFlags(0x06); // BR_EDR_NOT_SUPPORTED | GENERAL_DISC_MODE

    advData.setServiceData(BLEUUID((uint16_t)0xFEAA), serviceData);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
    pAdvertising->setMinInterval(160); // 100ms
    pAdvertising->setMaxInterval(320); // 200ms

    pAdvertising->start();

    beaconActive = true;
    lastMaintenanceTime = millis();

    DEBUG_PRINTLN("BLE beacon started successfully");
}


void BLEManager::stopBeacon() {
    if (pAdvertising) pAdvertising->stop();
    beaconActive = false;
    currentUrl = "";
}

void BLEManager::maintainBeacon() {
    if (!initialized || !beaconActive) return;

    if (millis() - lastMaintenanceTime > 30000) {
        if (pAdvertising) {
            pAdvertising->stop();
            delay(100);
            pAdvertising->start();
        }
        lastMaintenanceTime = millis();
    }
}

void BLEManager::cleanup() {
    if (beaconActive) stopBeacon();
    if (initialized) {
        BLEDevice::deinit(false);
        initialized = false;
    }
}

String BLEManager::encodeUrl(const String& url) {
    String encoded = "";
    String workingUrl = url;

    uint8_t schemePrefix = getUrlSchemePrefix(workingUrl);
    encoded += (char)schemePrefix;

    if (workingUrl.startsWith("http://www.")) workingUrl = workingUrl.substring(11);
    else if (workingUrl.startsWith("https://www.")) workingUrl = workingUrl.substring(12);
    else if (workingUrl.startsWith("http://")) workingUrl = workingUrl.substring(7);
    else if (workingUrl.startsWith("https://")) workingUrl = workingUrl.substring(8);

    workingUrl = compressUrl(workingUrl);
    encoded += workingUrl;

    return encoded;
}

uint8_t BLEManager::getUrlSchemePrefix(const String& url) {
    if (url.startsWith("http://www.")) return 0x00;
    if (url.startsWith("https://www.")) return 0x01;
    if (url.startsWith("http://")) return 0x02;
    if (url.startsWith("https://")) return 0x03;
    return 0x03;  // Default to https://
}

String BLEManager::compressUrl(const String& url) {
    String compressed = url;
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
    return compressed;
}
