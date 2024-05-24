#include<Arduino.h>
#include <Preferences.h>

#ifndef USER_PREFERENCES
#define USER_PREFERENCES

struct UserSettings {
  String ssid;
  String pass;
};

class UserPreferences {

  private:
    Preferences preferences;
    UserSettings settings;

  public:
    std::pair<UserSettings, bool> getPreferences();
    void setPreferences(UserSettings userSettings);
    void deletePreferences(void);
    void reset(void);

};

#endif

