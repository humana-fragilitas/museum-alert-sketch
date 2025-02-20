#include "cert_manager.h"

bool CertManager::store(Certificates certificates) {

  Preferences preferences;
  bool success;

  if ((success = preferences.begin(Storage::NAME, false))) {

    // Encrypt and store the certificates
    String encryptedClientCert = Ciphering::aes128Encrypt(certificates.clientCert);
    String encryptedPrivateKey = Ciphering::aes128Encrypt(certificates.privateKey);

    preferences.putString(Storage::CLIENT_CERT_LABEL, encryptedClientCert);
    preferences.putString(Storage::PRIVATE_KEY_LABEL, encryptedPrivateKey);
    preferences.end();

    DEBUG_PRINTLN("Encrypted and stored client TLS certificate and private key");
    DEBUG_PRINTF("Encrypted TLS certificate: %s\n", encryptedClientCert.c_str());
    DEBUG_PRINTF("Encrypted private key: %s\n", encryptedPrivateKey.c_str());

  } else {

    DEBUG_PRINTLN("Cannot store client TLS certificate and private key");

  }

  return success;

}

void CertManager::eraseCertificates() {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    preferences.clear();
    preferences.end();
    DEBUG_PRINTLN("Previously stored TLS certificate and private key have been erased");
  } else {
    DEBUG_PRINTLN("Cannot open and erase TLS certificate and private key storage");
  }

}

Certificates CertManager::retrieveCertificates() {

  Certificates certificates;
  Preferences preferences;

  if (!preferences.begin(Storage::NAME, true)) {
      DEBUG_PRINTLN("Failed to open TLS certificate and private key storage");
      return certificates;  // Return empty struct if storage cannot be opened
  }

  String encryptedClientCert;
  String encryptedPrivateKey;

  encryptedClientCert = preferences.getString(Storage::CLIENT_CERT_LABEL);
  encryptedPrivateKey = preferences.getString(Storage::PRIVATE_KEY_LABEL);
  preferences.end();

  // 🟢 Log heap before decryption
  DEBUG_PRINTF("Free heap before decryption: %d bytes\n", ESP.getFreeHeap());
  
  // 🟢 Log encrypted values (if they are valid)
  DEBUG_PRINTF("Encrypted Client Cert Length: %d\n", encryptedClientCert.length());
  DEBUG_PRINTF("Encrypted Private Key Length: %d\n", encryptedPrivateKey.length());

  // 🛑 Crash could be inside aes128Decrypt(), let's log before and after!
  DEBUG_PRINTLN("Starting decryption...");

  certificates.clientCert = Ciphering::aes128Decrypt(encryptedClientCert);
  DEBUG_PRINTLN("Client Cert decrypted successfully");

  certificates.privateKey = Ciphering::aes128Decrypt(encryptedPrivateKey);
  DEBUG_PRINTLN("Private Key decrypted successfully");

  // 🟢 Log heap after decryption
  DEBUG_PRINTF("Free heap after decryption: %d bytes\n", ESP.getFreeHeap());

  // 🟢 Validate decrypted values
  DEBUG_PRINTF("Decrypted Client Cert Length: %d\n", certificates.clientCert.length());
  DEBUG_PRINTF("Decrypted Private Key Length: %d\n", certificates.privateKey.length());

  if (certificates.clientCert.isEmpty() || certificates.privateKey.isEmpty()) {
      DEBUG_PRINTLN("Decryption failed! Possibly corrupted data.");
      return Certificates(); // Return empty struct to avoid crashes
  }

  DEBUG_PRINTLN("Retrieved TLS certificate and private key");
  return certificates;

}
