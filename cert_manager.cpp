#include "cert_manager.h"

void CertManager::storeCertificates(Certificates certificates) {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {

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

  certificates.clientCert = Ciphering::aes128Decrypt(encryptedClientCert);
  certificates.privateKey = Ciphering::aes128Decrypt(encryptedPrivateKey);

  DEBUG_PRINTLN("Retrieved TLS certificate and private key");
  DEBUG_PRINTF("Decrypted TLS certificate: %s\n", certificates.clientCert.c_str());
  DEBUG_PRINTF("Decrypted private key: %s\n", certificates.privateKey.c_str());

  return certificates;

}
