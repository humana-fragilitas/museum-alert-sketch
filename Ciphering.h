#ifndef CIPHERING_H
#define CIPHERING_H

#include <Arduino.h>
#include <Preferences.h>
#include <mbedtls/aes.h>
#include <esp_system.h>

#include "settings.h"
#include "helpers.h"
#include "macros.h"

class Ciphering {

  private:
    static uint8_t aes128Key[Encryption::KEY_SIZE];
    static bool aes128GenerateKey();
    static bool aes128RetrieveKey();
    static void aes128GenerateIV(uint8_t* iv);

  public:
    static bool initialize();
    static void aes128Encrypt(const char *input, char *output);
    static void aes128Decrypt(const char *input, char *output);
};

#endif
