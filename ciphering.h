#ifndef CIPHERING_H
#define CIPHERING_H

#include <Arduino.h>
#include <Preferences.h>
#include <mbedtls/aes.h>
#include <esp_system.h>

#include "config.h"
#include "helpers.h"

class Ciphering {

  private:
    static uint8_t aes128Key[Encryption::KEY_SIZE];
    static bool aes128GenerateKey();
    static bool aes128RetrieveKey();
    static void aes128GenerateIV(uint8_t* iv) noexcept;

  public:
    static bool initialize();
    static String aes128Encrypt(const String& input);
    static String aes128Decrypt(const String& input);

};

#endif
