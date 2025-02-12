#include "cert_manager.h"

void CertManager::storeCertificates(Certificates certificates) {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    
    char encryptedClientCert[Certificates::CERT_SIZE * 2 + 1] = {0};
    char encryptedPrivateKey[Certificates::KEY_SIZE * 2 + 1] = {0};

    // Encrypt and store the certificates
    Ciphering::aes128Encrypt(certificates.clientCert, encryptedClientCert);
    Ciphering::aes128Encrypt(certificates.privateKey, encryptedPrivateKey);

    preferences.putBytes(Storage::CLIENT_CERT_LABEL, encryptedClientCert, sizeof(encryptedClientCert));
    preferences.putBytes(Storage::PRIVATE_KEY_LABEL, encryptedPrivateKey, sizeof(encryptedPrivateKey));
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
        DEBUG_PRINTLN("Failed to open preferences storage");
        return certificates;  // Return empty struct if storage cannot be opened
    }

    // char encryptedClientCert[Certificates::CERT_SIZE * 2 + 1] = {0};
    // char encryptedPrivateKey[Certificates::KEY_SIZE * 2 + 1] = {0};

    // size_t certSize = preferences.getBytesLength(Storage::CLIENT_CERT_LABEL);
    // size_t keySize = preferences.getBytesLength(Storage::PRIVATE_KEY_LABEL);

    // if (certSize > 0 && certSize < sizeof(encryptedClientCert)) {
    //     preferences.getBytes(Storage::CLIENT_CERT_LABEL, encryptedClientCert, certSize);
    // } else {
    //     DEBUG_PRINTLN("Invalid client certificate size!");
    // }

    // if (keySize > 0 && keySize < sizeof(encryptedPrivateKey)) {
    //     preferences.getBytes(Storage::PRIVATE_KEY_LABEL, encryptedPrivateKey, keySize);
    // } else {
    //     DEBUG_PRINTLN("Invalid private key size!");
    // }

    // preferences.end();

    // memset(certificates.clientCert, 0, Certificates::CERT_SIZE);
    // memset(certificates.privateKey, 0, Certificates::KEY_SIZE);

    // Ciphering::aes128Decrypt(encryptedClientCert, certificates.clientCert);
    // Ciphering::aes128Decrypt(encryptedPrivateKey, certificates.privateKey);

    // DEBUG_PRINTLN("Retrieved client TLS certificate and private key");
    return certificates;
}
