#include "storage_manager.h"

void StorageManager::erase() noexcept {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    preferences.clear();
    preferences.end();
    DEBUG_PRINTLN("Previously stored TLS certificate, private key and company name have been erased");
  } else {
    DEBUG_PRINTLN("Cannot open and erase TLS certificate, private key and company name storage");
  }

}

template<>
bool StorageManager::save<Distance>(const Distance& value) {

  Preferences preferences;
  bool success{ false };

  // Note: assignment and evaluation
  if ((success = preferences.begin(Storage::NAME, false))) {

    preferences.putFloat(Storage::DISTANCE_LABEL, value);
    preferences.end();

    DEBUG_PRINTF("Stored minimum alarm distance: %f\n", value);

  } else {

    DEBUG_PRINTLN("Cannot store minimum alarm distance");
  }

  return success;

}

template<>
bool StorageManager::save<BeaconURL>(const BeaconURL& value) {

  Preferences preferences;
  bool success{ false };

  // Note: assignment and evaluation
  if ((success = preferences.begin(Storage::NAME, false))) {

    preferences.putString(Storage::BEACON_URL_LABEL, value);
    preferences.end();

    DEBUG_PRINTF("Stored beacon URL: %s\n",
                 value.isEmpty() ? "(empty)" : value.c_str());

  } else {

    DEBUG_PRINTLN("Cannot store beacon URL");
  }

  return success;

}

template<>
bool StorageManager::save<AwsIotConfiguration>(const AwsIotConfiguration& value) {

  Preferences preferences;
  bool success{ false };

  // Note: assignment and evaluation
  if ((success = preferences.begin(Storage::NAME, false))) {

    const auto encryptedClientCert = Ciphering::aes128Encrypt(value.certificates.clientCert);
    const auto encryptedPrivateKey = Ciphering::aes128Encrypt(value.certificates.privateKey);
    const auto encryptedCompanyName = Ciphering::aes128Encrypt(value.companyName);

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

}

template<>
Distance StorageManager::load<Distance>() {

  Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
    DEBUG_PRINTF("Failed to open minimum alarm distance storage; defaulting to %f cm\n", Configuration::DEFAULT_ALARM_DISTANCE);
    return Configuration::DEFAULT_ALARM_DISTANCE;
  }

  if (!preferences.isKey(Storage::DISTANCE_LABEL)) {
    DEBUG_PRINTF("No stored value found for minimum alarm distance; defaulting to %f cm\n", Configuration::DEFAULT_ALARM_DISTANCE);
    preferences.end();
    return Configuration::DEFAULT_ALARM_DISTANCE;
  }

  const auto minimumAlarmDistance = preferences.getFloat(Storage::DISTANCE_LABEL);
  preferences.end();

  DEBUG_PRINTF("Retrieved minimum alarm distance from storage: %f\n", minimumAlarmDistance);

  return minimumAlarmDistance;

}

template<>
BeaconURL StorageManager::load<BeaconURL>() {

  Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
    DEBUG_PRINTF("Failed to open beacon URL storage; sensor will not broadcast any beacon\n");
    return "";
  }

  if (!preferences.isKey(Storage::BEACON_URL_LABEL)) {
    DEBUG_PRINTLN("No stored value found for beacon URL; sensor will not broadcast any beacon");
    preferences.end();
    return "";
  }

  const auto url = preferences.getString(Storage::BEACON_URL_LABEL);
  preferences.end();

  if (url.isEmpty()) {

    DEBUG_PRINTF("Retrieved empty beacon URL from storage; sensor will not broadcast any beacon\n");

  } else {

    DEBUG_PRINTF("Retrieved beacon URL from storage: %s\n", url.c_str());
  }

  return url;

}

template<>
AwsIotConfiguration StorageManager::load<AwsIotConfiguration>() {

  AwsIotConfiguration configuration{};
  Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
    DEBUG_PRINTLN("Failed to open TLS certificate and private key storage");
    return configuration;
  }

  const auto encryptedClientCert = preferences.getString(Storage::CLIENT_CERT_LABEL);
  const auto encryptedPrivateKey = preferences.getString(Storage::PRIVATE_KEY_LABEL);
  const auto encryptedCompanyName = preferences.getString(Storage::COMPANY_NAME_LABEL);
  preferences.end();

  // Log heap before decryption
  DEBUG_PRINTF("Free heap before decryption: %d bytes\n", ESP.getFreeHeap());

  // Log encrypted values
  DEBUG_PRINTF("Encrypted client cert length: %d\n", encryptedClientCert.length());
  DEBUG_PRINTF("Encrypted private key length: %d\n", encryptedPrivateKey.length());
  DEBUG_PRINTF("Encrypted company name length: %d\n", encryptedCompanyName.length());

  DEBUG_PRINTLN("Starting decryption...");

  configuration.certificates.clientCert = Ciphering::aes128Decrypt(encryptedClientCert);

  configuration.certificates.privateKey = Ciphering::aes128Decrypt(encryptedPrivateKey);

  configuration.companyName = Ciphering::aes128Decrypt(encryptedCompanyName);

  // Log heap after decryption
  DEBUG_PRINTF("Free heap after decryption: %d bytes\n", ESP.getFreeHeap());

  // Validate decrypted values
  DEBUG_PRINTF("Decrypted client cert length: %d\n", configuration.certificates.clientCert.length());
  DEBUG_PRINTF("Decrypted private key length: %d\n", configuration.certificates.privateKey.length());
  DEBUG_PRINTF("Decrypted company name length: %d\n", configuration.companyName.length());

  if (configuration.certificates.clientCert.isEmpty() || configuration.certificates.privateKey.isEmpty() || configuration.companyName.isEmpty()) {
    DEBUG_PRINTLN("Decryption failed! Possibly corrupted data.");
    return configuration;
  }

  DEBUG_PRINTLN("Retrieved TLS certificate and private key");

  return configuration;

}
