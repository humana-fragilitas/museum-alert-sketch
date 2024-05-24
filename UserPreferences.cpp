#include <utility>
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <nvs_flash.h>

#include "UserPreferences.h"

std::pair<UserSettings, bool> UserPreferences::getPreferences() {

  bool hasPreferences;

  preferences.begin("preferences", false);

  String ssid = preferences.getString("ssid");
  String pass = preferences.getString("pass");

  preferences.end();

  if (hasPreferences = (ssid != nullptr && pass != nullptr)) {

    Serial.println("\nUser preferences found on this device");

    settings.ssid = ssid;
    settings.pass = pass;
    
  } else {

    Serial.println("\nNo user preferences found on this device");

  }

  return std::make_pair(settings, hasPreferences);

}

void UserPreferences::setPreferences(UserSettings userSettings) {
  
  preferences.begin("preferences", false);

  preferences.putString("ssid", userSettings.ssid);
  preferences.putString("pass", userSettings.pass);

  preferences.end();

  Serial.printf("\nUser preferences set: ssid: %s; pass: %s", userSettings.ssid.c_str(), userSettings.pass.c_str());

}

void UserPreferences::deletePreferences(void) {

  preferences.begin("preferences", false);

  preferences.clear();
  Serial.println("User preferences deleted");

  preferences.end();

}

void UserPreferences::reset(void) {

  nvs_flash_erase();
  nvs_flash_init();

}






