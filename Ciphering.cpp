#include "Ciphering.h"

// Generate a new IV securely
void Ciphering::generateIV(uint8_t* iv) {
  esp_fill_random(iv, AES_BLOCK_SIZE); // ESP32's hardware RNG
}

String Ciphering::aes128Encrypt(const String& input) {

  size_t length = input.length();
  uint8_t iv[AES_BLOCK_SIZE];
  generateIV(iv);

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128);

  uint8_t* output = new uint8_t[length + AES_BLOCK_SIZE];
  memcpy(output, iv, AES_BLOCK_SIZE); // Store IV in the first 16 bytes

  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, iv, (uint8_t*)input.c_str(), output + AES_BLOCK_SIZE);
  mbedtls_aes_free(&aes);

  String result;
  for (size_t i = 0; i < length + AES_BLOCK_SIZE; ++i) {
      result += String(output[i], HEX);
  }

  delete[] output;

  return result;

}

String Ciphering::aes128Decrypt(const String& input) {

  size_t length = input.length() / 2; // Convert hex string length to byte length

  uint8_t* encryptedData = new uint8_t[length];
  for (size_t i = 0; i < length; ++i) {
      sscanf(input.substring(2 * i, 2 * i + 2).c_str(), "%02x", &encryptedData[i]);
  }

  uint8_t iv[AES_BLOCK_SIZE];
  memcpy(iv, encryptedData, AES_BLOCK_SIZE); // Extract IV from first 16 bytes

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, key, 128);

  uint8_t* output = new uint8_t[length - AES_BLOCK_SIZE];
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length - AES_BLOCK_SIZE, iv, encryptedData + AES_BLOCK_SIZE, output);
  mbedtls_aes_free(&aes);

  String result = String((char*)output);
  delete[] encryptedData;
  delete[] output;

  return result;

}

const uint8_t Ciphering::key[Ciphering::KEY_SIZE] = {
  0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
  0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81
};