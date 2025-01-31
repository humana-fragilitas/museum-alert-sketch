#include <Arduino.h>
#include <mbedtls/aes.h>
#include <esp_system.h> 

#ifndef CIPHERING
#define CIPHERING

class Ciphering {

  private:
    static constexpr size_t KEY_SIZE = 16; // 128-bit key
    static constexpr size_t AES_BLOCK_SIZE = 16; // AES block size is always 16 bytes
    // AES Key (must be 16 bytes for AES-128)
    static const uint8_t key[KEY_SIZE];
    static void generateIV(uint8_t* iv);

  public:
    static String aes128Encrypt(const String& input);
    static String aes128Decrypt(const String& input);

};

#endif