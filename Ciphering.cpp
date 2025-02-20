#include "ciphering.h"

uint8_t Ciphering::aes128Key[Encryption::KEY_SIZE] = {0};

void Ciphering::aes128GenerateIV(uint8_t* iv) {
  esp_fill_random(iv, Encryption::AES_BLOCK_SIZE);
}

String Ciphering::aes128Encrypt(String input) {
    if (input.length() == 0) {
        return "";
    }

    const uint8_t* inputData = reinterpret_cast<const uint8_t*>(input.c_str());
    size_t inputLength = input.length();
    
    // Calculate padded length
    size_t paddedLength = ((inputLength + Encryption::AES_BLOCK_SIZE - 1) / Encryption::AES_BLOCK_SIZE) 
                         * Encryption::AES_BLOCK_SIZE;

    // Debug original data
    DEBUG_PRINT("Original data starts with: ");
    for (size_t i = 0; i < std::min(size_t(32), inputLength); i++) {
        DEBUG_PRINTF("%02X ", inputData[i]);
    }
    DEBUG_PRINTLN("");

    // Prepare buffers
    uint8_t iv[Encryption::AES_BLOCK_SIZE];
    std::vector<uint8_t> buffer(paddedLength + Encryption::AES_BLOCK_SIZE);  // Extra block for IV

    // Generate IV
    Ciphering::aes128GenerateIV(iv);
    
    // Copy IV to start of buffer
    memcpy(buffer.data(), iv, Encryption::AES_BLOCK_SIZE);
    
    // Copy input data after IV
    memcpy(buffer.data() + Encryption::AES_BLOCK_SIZE, inputData, inputLength);

    // Apply PKCS#7 padding
    uint8_t paddingValue = paddedLength - inputLength;
    for (size_t i = inputLength + Encryption::AES_BLOCK_SIZE; i < buffer.size(); i++) {
        buffer[i] = paddingValue;
    }

    // Initialize AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, aes128Key, 128) != 0) {
        DEBUG_PRINTLN("Error setting encryption key");
        mbedtls_aes_free(&aes);
        return "";
    }

    // Encrypt (skip the IV block)
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLength,
                             iv, buffer.data() + Encryption::AES_BLOCK_SIZE,
                             buffer.data() + Encryption::AES_BLOCK_SIZE) != 0) {
        DEBUG_PRINTLN("Encryption error");
        mbedtls_aes_free(&aes);
        return "";
    }
    
    mbedtls_aes_free(&aes);

    // Build output string with hex representation (including IV)
    String output;
    output.reserve(buffer.size() * 2);

    // Convert entire buffer to hex (IV + encrypted data)
    for (size_t i = 0; i < buffer.size(); i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", buffer[i]);
        output += hex;
    }

    return output;
}

String Ciphering::aes128Decrypt(String input) {
    if (input.length() < 2 * Encryption::AES_BLOCK_SIZE) {
        DEBUG_PRINTLN("Invalid ciphertext: too short.");
        return "";
    }

    size_t encryptedLength = input.length() / 2;
    
    // Prepare buffer for entire encrypted data including IV
    std::vector<uint8_t> buffer(encryptedLength);

    // Parse hex string to bytes
    for (size_t i = 0; i < encryptedLength; i++) {
        unsigned int byte;
        if (sscanf(input.c_str() + (i * 2), "%02x", &byte) != 1) {
            DEBUG_PRINTLN("Error parsing encrypted data");
            return "";
        }
        buffer[i] = (uint8_t)byte;
    }

    // Extract IV from the first block
    uint8_t iv[Encryption::AES_BLOCK_SIZE];
    memcpy(iv, buffer.data(), Encryption::AES_BLOCK_SIZE);

    // Initialize AES
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_dec(&aes, aes128Key, 128) != 0) {
        DEBUG_PRINTLN("Error setting decryption key");
        mbedtls_aes_free(&aes);
        return "";
    }

    // Decrypt (skip the IV block)
    size_t dataLength = encryptedLength - Encryption::AES_BLOCK_SIZE;
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, dataLength,
                             iv, buffer.data() + Encryption::AES_BLOCK_SIZE,
                             buffer.data() + Encryption::AES_BLOCK_SIZE) != 0) {
        DEBUG_PRINTLN("Decryption error");
        mbedtls_aes_free(&aes);
        return "";
    }
    
    mbedtls_aes_free(&aes);

    // Debug first bytes of decrypted data
    DEBUG_PRINT("Decrypted data starts with: ");
    for (size_t i = 0; i < std::min(size_t(32), dataLength); i++) {
        DEBUG_PRINTF("%02X ", buffer[i + Encryption::AES_BLOCK_SIZE]);
    }
    DEBUG_PRINTLN("");

    // Remove PKCS#7 padding
    if (dataLength > 0) {
        uint8_t paddingValue = buffer[encryptedLength - 1];
        if (paddingValue > 0 && paddingValue <= Encryption::AES_BLOCK_SIZE) {
            bool validPadding = true;
            for (size_t i = encryptedLength - paddingValue; i < encryptedLength; i++) {
                if (buffer[i] != paddingValue) {
                    validPadding = false;
                    break;
                }
            }
            if (validPadding) {
                dataLength -= paddingValue;
            }
        }
    }

    // Create output string from decrypted data (excluding IV)
    String output;
    output.reserve(dataLength);
    output.concat(reinterpret_cast<char*>(buffer.data() + Encryption::AES_BLOCK_SIZE), dataLength);

    return output;
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
