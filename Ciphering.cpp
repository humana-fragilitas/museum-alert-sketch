#include "ciphering.h"

uint8_t Ciphering::aes128Key[Encryption::KEY_SIZE] = {0};

void Ciphering::aes128GenerateIV(uint8_t* iv) {
  esp_fill_random(iv, Encryption::AES_BLOCK_SIZE);
}

String Ciphering::aes128Encrypt(String input) {
    size_t inputLength = input.length();
    size_t paddedLength = ((inputLength / Encryption::AES_BLOCK_SIZE) + 1) * Encryption::AES_BLOCK_SIZE;

    // Buffers
    uint8_t iv[Encryption::AES_BLOCK_SIZE] = {0};
    std::vector<uint8_t> buffer(paddedLength, 0);

    // Copy input and apply PKCS#7 padding
    memcpy(buffer.data(), input.c_str(), inputLength);
    uint8_t pad = Encryption::AES_BLOCK_SIZE - (inputLength % Encryption::AES_BLOCK_SIZE);
    memset(buffer.data() + inputLength, pad, pad);

    // Initialize AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aes128Key, 128);

    Ciphering::aes128GenerateIV(iv);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLength, iv, buffer.data(), buffer.data());
    mbedtls_aes_free(&aes);

    // Convert to hexadecimal string
    String output;
    for (size_t i = 0; i < paddedLength; ++i) {
        char hex[3];
        sprintf(hex, "%02X", buffer[i]);
        output += hex;
    }

    return output;
}

String Ciphering::aes128Decrypt(String input) {
    size_t length = input.length() / 2;

    // Buffers
    std::vector<uint8_t> encryptedData(length, 0);
    uint8_t iv[Encryption::AES_BLOCK_SIZE] = {0};
    std::vector<uint8_t> buffer(length, 0);

    // Convert hex string to bytes
    for (size_t i = 0; i < length; ++i) {
        sscanf(input.c_str() + (i * 2), "%02x", &encryptedData[i]);
    }

    // Initialize AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, aes128Key, 128);

    // Decrypt
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length, iv, encryptedData.data(), buffer.data());
    mbedtls_aes_free(&aes);

    // Remove PKCS#7 padding
    uint8_t pad = buffer[length - 1];
    if (pad > 0 && pad <= Encryption::AES_BLOCK_SIZE) {
        buffer[length - pad] = '\0';
    }

    return String(reinterpret_cast<char *>(buffer.data()));
}


bool Ciphering::aes128GenerateKey() {

  DEBUG_PRINTLN("Generating new encryption key...");

  uint8_t tempKey[Encryption::KEY_SIZE];
  esp_fill_random(tempKey, Encryption::KEY_SIZE);

  Preferences preferences;
  if (!preferences.begin(Storage::NAME, false)) {
    DEBUG_PRINTLN("Error while generating ciphering key: unable to open preferences for writing");
    return false;
  }

  size_t size = preferences.putBytes(Storage::ENCRYPTION_KEY_LABEL, tempKey, Encryption::KEY_SIZE);
  preferences.end();

  if (size == Encryption::KEY_SIZE) {
    memcpy(aes128Key, tempKey, Encryption::KEY_SIZE);
    DEBUG_PRINTLN("Encryption key successfully stored");
    return true;
  } else {
    DEBUG_PRINTLN("Failed to store encryption key");
    return false;
  }

}

bool Ciphering::aes128RetrieveKey() {

  Preferences preferences;
  bool keyIsValid = false;

  if (preferences.begin(Storage::NAME, true)) {

    DEBUG_PRINTLN("Retrieving encryption key from storage...");
    size_t size = preferences.getBytes(Storage::ENCRYPTION_KEY_LABEL, aes128Key, Encryption::KEY_SIZE);
    preferences.end();

    if (size == Encryption::KEY_SIZE) {
        keyIsValid = true;
    }

  }

  if (keyIsValid) {
    DEBUG_PRINTLN("Successfully retrieved encryption key.");
  } else {
    DEBUG_PRINTLN("Failed to retrieve encryption key.");
  }

  return keyIsValid;

}

bool Ciphering::initialize() {

  return Ciphering::aes128RetrieveKey() || Ciphering::aes128GenerateKey();

}
