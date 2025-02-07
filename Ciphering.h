#include <Arduino.h>
#include <Preferences.h>
#include <mbedtls/aes.h>
#include <esp_system.h>

#include "Configuration.h"
#include "Helpers.h"
#include "Macros.h"

#ifndef CIPHERING
#define CIPHERING

class Ciphering {

  private:
    static std::vector<uint8_t> aes128Key;
    static bool aes128HasKey();
    static bool aes128GenerateKey();
    static bool aes128RetrieveKey();
    static void aes128GenerateIV(uint8_t* iv);

  public:
    static bool initialize();
    static String aes128Encrypt(const String& input);
    static String aes128Decrypt(const String& input);

};

#endif