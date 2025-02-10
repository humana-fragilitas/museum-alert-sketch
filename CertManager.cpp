#include "CertManager.h"

void CertManager::storeCertificates(Certificates& certificates) {

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    
    char encryptedClientCert[Certificates::CERT_SIZE * 2 + 1] = {0};
    char encryptedPrivateKey[Certificates::KEY_SIZE * 2 + 1] = {0};

    char tempClientCert[Certificates::CERT_SIZE];
    char tempPrivateKey[Certificates::KEY_SIZE];

    certificates.getClientCert(tempClientCert, Certificates::CERT_SIZE);
    certificates.getPrivateKey(tempPrivateKey, Certificates::KEY_SIZE);

    // Encrypt and store the certificates
    Ciphering::aes128Encrypt(tempClientCert, encryptedClientCert);
    Ciphering::aes128Encrypt(tempPrivateKey, encryptedPrivateKey);

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

    if (preferences.begin(Storage::NAME, true)) {
        char encryptedClientCert[Certificates::CERT_SIZE * 2 + 1] = {0};
        char encryptedPrivateKey[Certificates::KEY_SIZE * 2 + 1] = {0};

        // Retrieve encrypted certificates
        preferences.getBytes(Storage::CLIENT_CERT_LABEL, encryptedClientCert, sizeof(encryptedClientCert));
        preferences.getBytes(Storage::PRIVATE_KEY_LABEL, encryptedPrivateKey, sizeof(encryptedPrivateKey));
        preferences.end();

        char tempClientCert[Certificates::CERT_SIZE];
        char tempPrivateKey[Certificates::KEY_SIZE];

        certificates.getClientCert(tempClientCert, Certificates::CERT_SIZE);
        certificates.getPrivateKey(tempPrivateKey, Certificates::KEY_SIZE);

        // Decrypt certificates
        Ciphering::aes128Decrypt(encryptedClientCert, tempClientCert);
        Ciphering::aes128Decrypt(encryptedPrivateKey, tempPrivateKey);

        DEBUG_PRINTLN("Retrieved client TLS certificate and private key");
    } else {
        DEBUG_PRINTLN("Failed to retrieve client TLS certificate and private key");
    }

    return certificates;
}