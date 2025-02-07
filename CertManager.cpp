#include "CertManager.h"

void CertManager::storeCertificates(Certificates certificates) {

  Preferences preferences;

  if(preferences.begin(Storage::NAME, false)) {

    preferences.putString(Storage::CLIENT_CERT_LABEL, Ciphering::aes128Encrypt(certificates.clientCert));
    preferences.putString(Storage::PRIVATE_KEY_LABEL, Ciphering::aes128Encrypt(certificates.privateKey));
    preferences.end();

    DEBUG_PRINTLN("Encrypted and stored client TLS certificate and private key");

  } else {

    DEBUG_PRINTLN("Cannot store client TLS certificate and private key");

  }

}

void CertManager::eraseCertificates() {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {

    preferences.clear();

    DEBUG_PRINTLN("Previously stored TLS certificate and private key have been erased");

    preferences.end();

  } else {

    DEBUG_PRINTLN("Cannot open and erase TLS certificate and private key storage");

  }

}

Certificates CertManager::retrieveCertificates() {

  Certificates certificates;
  Preferences preferences;

  if (preferences.begin(Storage::NAME, true)) {

    certificates.clientCert = Ciphering::aes128Decrypt(preferences.getString(Storage::CLIENT_CERT_LABEL));
    certificates.privateKey = Ciphering::aes128Decrypt(preferences.getString(Storage::PRIVATE_KEY_LABEL));
    preferences.end();

    DEBUG_PRINTLN("Retrieved client TLS certificate and private key");

  } else {

    DEBUG_PRINTLN("Failed to retrieve client TLS certificate and private key");

  }

  return certificates;

}

// TO DO: replace with constant entries in the configuration file
/*const char CertManager::aesKey[] = "1234567890123456"; // 16-byte key for AES-128
const char CertManager::aesIV[] = "abcdefghijklmnop"; // 16-byte IV for CBC mode
const char CertManager::storageNamespace[] = "STORAGE";
const char CertManager::clientCertStorageLabel[] = "CLIENT_CERT";
const char CertManager::privateKeyStorageLabel[] = "PRIVATE_KEY";*/