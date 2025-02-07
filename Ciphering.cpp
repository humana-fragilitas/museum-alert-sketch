#include "Ciphering.h"

void Ciphering::aes128GenerateIV(uint8_t* iv) {
  esp_fill_random(iv, Encryption::AES_BLOCK_SIZE);
}

String Ciphering::aes128Encrypt(const String& input) {

  size_t length = input.length();
  uint8_t iv[Encryption::AES_BLOCK_SIZE];
  aes128GenerateIV(iv);

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, aes128Key.data(), 128);

  uint8_t* output = new uint8_t[length + Encryption::AES_BLOCK_SIZE];
  memcpy(output, iv, Encryption::AES_BLOCK_SIZE); // Store IV in the first 16 bytes

  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT,
    length, iv, (uint8_t*)input.c_str(), output + Encryption::AES_BLOCK_SIZE);
  mbedtls_aes_free(&aes);

  String result;
  for (size_t i = 0; i < length + Encryption::AES_BLOCK_SIZE; ++i) {
      result += String(output[i], HEX);
  }

  delete[] output;

  DEBUG_PRINTF("Encryption required for the following string: %s\n", input);
  DEBUG_PRINTF("Encrypted output: %s\n", result);

  return result;

}

String Ciphering::aes128Decrypt(const String& input) {

  size_t length = input.length() / 2; // Convert hex string length to byte length

  uint8_t* encryptedData = new uint8_t[length];
  for (size_t i = 0; i < length; ++i) {
      sscanf(input.substring(2 * i, 2 * i + 2).c_str(), "%02x", &encryptedData[i]);
  }

  uint8_t iv[Encryption::AES_BLOCK_SIZE];
  memcpy(iv, encryptedData, Encryption::AES_BLOCK_SIZE); // Extract IV from first 16 bytes

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, aes128Key.data(), 128);

  uint8_t* output = new uint8_t[length - Encryption::AES_BLOCK_SIZE];
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT,
    length - Encryption::AES_BLOCK_SIZE, iv, encryptedData + Encryption::AES_BLOCK_SIZE, output);
  mbedtls_aes_free(&aes);

  String result = String((char*)output);
  delete[] encryptedData;
  delete[] output;

  DEBUG_PRINTF("Decryption required for the following string: %s\n", input);
  DEBUG_PRINTF("Decrypted output: %s\n", result);

  return result;

}

bool Ciphering::aes128HasKey() {

  Preferences preferences;
  bool hasKey = false;

  preferences.begin(Storage::NAME, true);
  hasKey = preferences.isKey(Storage::ENCRYPTION_KEY_LABEL);
  preferences.end();

  DEBUG_PRINTLN(hasKey ? "Found storage label of previously saved encryption key" :
    "Cannot find storage label of previously saved encryption key");

  return hasKey;
  
}

bool Ciphering::aes128GenerateKey() {

  DEBUG_PRINTLN("Creating encryption key...");

  std::vector<uint8_t> aes128TempKey(Encryption::KEY_SIZE);
  esp_fill_random(aes128TempKey.data(), Encryption::KEY_SIZE);

  Preferences preferences;

  if (preferences.begin(Storage::NAME, false)) {
    
    DEBUG_PRINTLN("Cannot open storage in read/write mode to store encryption key");
    return false;

  }

  size_t size = preferences.putBytes(Storage::ENCRYPTION_KEY_LABEL, aes128TempKey.data(), Encryption::KEY_SIZE);

  if (size == Encryption::KEY_SIZE) {

    aes128Key = aes128TempKey;
    DEBUG_PRINTLN("Successfully stored encryption key");

  } else {

    return false;

  }

  preferences.end();

  return true;  

}

bool Ciphering::initialize() {

  return Ciphering::aes128HasKey() ?
    (Ciphering::aes128RetrieveKey() || Ciphering::aes128GenerateKey()) : false;

}

bool Ciphering::aes128RetrieveKey() {

  Preferences preferences;
  bool keyIsValid = false;

  if (preferences.begin(Storage::NAME, true)) {

    DEBUG_PRINTLN("Retrieving encryption key from storage...");

    preferences.getBytes(Storage::ENCRYPTION_KEY_LABEL, aes128Key.data(), Encryption::KEY_SIZE);

    for (size_t i = 0; i < Encryption::KEY_SIZE; ++i) {
      if (aes128Key[i] != 0) {
        keyIsValid = true;
        break;
      }
    }

  }

  if (keyIsValid) {
    DEBUG_PRINTF("Successfully retrieved encryption key from storage: %s\n", encryptionKeyToHexString(aes128Key));
  } else {
    DEBUG_PRINTLN("Successfully retrieved encryption key from storage");
  }

  preferences.end();

  return keyIsValid;

}

std::vector<uint8_t> Ciphering::aes128Key(Encryption::KEY_SIZE, 0);