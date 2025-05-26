#include "storage_manager.h"

void StorageManager::erase(){

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    preferences.clear();
    preferences.end();
    DEBUG_PRINTLN("Previously stored TLS certificate, private key and company name have been erased");
  } else {
    DEBUG_PRINTLN("Cannot open and erase TLS certificate, private key and company name storage");
  }

};

template<>
bool StorageManager::save<Distance>(const Distance& value){

  Preferences preferences;
  bool success;

  if ((success = preferences.begin(Storage::NAME, false))) {

    preferences.putFloat(Storage::DISTANCE_LABEL, value);
    preferences.end();

    DEBUG_PRINTF("Stored minimum alarm distance: %f\n", value);

  } else {

    DEBUG_PRINTLN("Cannot store minimum alarm distance");

  }

  return success;

};

template<>
bool StorageManager::save<DeviceConfiguration>(const DeviceConfiguration& value){
  
  Preferences preferences;
  bool success;

  if ((success = preferences.begin(Storage::NAME, false))) {

    String encryptedClientCert = Ciphering::aes128Encrypt(value.certificates.clientCert);
    String encryptedPrivateKey = Ciphering::aes128Encrypt(value.certificates.privateKey);
    String encryptedCompanyName = Ciphering::aes128Encrypt(value.companyName);

    preferences.putString(Storage::CLIENT_CERT_LABEL, encryptedClientCert);
    preferences.putString(Storage::PRIVATE_KEY_LABEL, encryptedPrivateKey);
    preferences.putString(Storage::COMPANY_NAME_LABEL, encryptedCompanyName);
    preferences.end();

    DEBUG_PRINTLN("Encrypted and stored client TLS certificate and private key");
    DEBUG_PRINTF("Encrypted TLS certificate: %s\n", encryptedClientCert.c_str());
    DEBUG_PRINTF("Encrypted private key: %s\n", encryptedPrivateKey.c_str());
    DEBUG_PRINTF("Encrypted company name: %s\n", encryptedCompanyName.c_str());

  } else {

    DEBUG_PRINTLN("Cannot store client TLS certificate, private key and company name");

  }

  return success;

};

template<>
Distance StorageManager::load<Distance>() {

   Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
      DEBUG_PRINTF("Failed to open minimum alarm distance storage; defaulting to %f cm\n", DEFAULT_ALARM_DISTANCE);
      return DEFAULT_ALARM_DISTANCE;
  }

  if (!preferences.isKey(Storage::DISTANCE_LABEL)) {
      DEBUG_PRINTF("No stored value found for minimum alarm distance; defaulting to %f cm\n", DEFAULT_ALARM_DISTANCE);
      preferences.end();
      return DEFAULT_ALARM_DISTANCE;
  }

  float minimumAlarmDistance = preferences.getFloat(Storage::DISTANCE_LABEL);
  preferences.end();

  DEBUG_PRINTF("Retrieved minimum alarm distance from storage: %f\n", minimumAlarmDistance);

  return minimumAlarmDistance;

};

template<>
DeviceConfiguration StorageManager::load<DeviceConfiguration>() {

  DeviceConfiguration configuration;
  Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
      DEBUG_PRINTLN("Failed to open TLS certificate and private key storage");
      return configuration;  // Return empty struct if storage cannot be opened
  }

  String encryptedClientCert;
  String encryptedPrivateKey;
  String encryptedCompanyName;

  encryptedClientCert = preferences.getString(Storage::CLIENT_CERT_LABEL);
  encryptedPrivateKey = preferences.getString(Storage::PRIVATE_KEY_LABEL);
  encryptedCompanyName = preferences.getString(Storage::COMPANY_NAME_LABEL);
  preferences.end();

  // 🟢 Log heap before decryption
  DEBUG_PRINTF("Free heap before decryption: %d bytes\n", ESP.getFreeHeap());
  
  // 🟢 Log encrypted values (if they are valid)
  DEBUG_PRINTF("Encrypted client cert length: %d\n", encryptedClientCert.length());
  DEBUG_PRINTF("Encrypted private key length: %d\n", encryptedPrivateKey.length());
  DEBUG_PRINTF("Encrypted company name length: %d\n", encryptedCompanyName.length());

  // 🛑 Crash could be inside aes128Decrypt(), let's log before and after!
  DEBUG_PRINTLN("Starting decryption...");

  configuration.certificates.clientCert = Ciphering::aes128Decrypt(encryptedClientCert);

  configuration.certificates.privateKey = Ciphering::aes128Decrypt(encryptedPrivateKey);

  configuration.companyName = Ciphering::aes128Decrypt(encryptedCompanyName);

  // 🟢 Log heap after decryption
  DEBUG_PRINTF("Free heap after decryption: %d bytes\n", ESP.getFreeHeap());

  // 🟢 Validate decrypted values
  DEBUG_PRINTF("Decrypted client cert length: %d\n", configuration.certificates.clientCert.length());
  DEBUG_PRINTF("Decrypted private key length: %d\n", configuration.certificates.privateKey.length());
  DEBUG_PRINTF("Decrypted company name length: %d\n", configuration.companyName.length());

  if (configuration.certificates.clientCert.isEmpty() ||
      configuration.certificates.privateKey.isEmpty() ||
      configuration.companyName.isEmpty()) {
      DEBUG_PRINTLN("Decryption failed! Possibly corrupted data.");
      return configuration; // Return empty struct to avoid crashes
  }

  DEBUG_PRINTLN("Retrieved TLS certificate and private key");
  
  return configuration;

};
