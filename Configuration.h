#include<Arduino.h>
#include <Preferences.h>

#ifndef CONFIGURATION
#define CONFIGURATION

struct WiFiCredentials {
  String ssid;
  String password;
  bool isValid() {
    return !ssid.isEmpty() && !password.isEmpty();
  }
};

struct ConnectionSettings {
  //String serverCert;
  String clientCert;
  String privateKey;
  String mqttEndpoint;
  bool isValid() {
    return !clientCert.isEmpty() &&
           !privateKey.isEmpty() &&
           !mqttEndpoint.isEmpty();
  }
};

class Configuration {

  private:
    Preferences preferences;
    ConnectionSettings provisioningSettings;

  public:
    ConnectionSettings getConnectionSettings();
    void setConnectionSettings(ConnectionSettings settings);
    void deleteConnectionSettings(void);
    ConnectionSettings getProvisioningConnectionSettings();
    void setProvisioningConnectionSettings(ConnectionSettings settings);
    void deleteProvisioningConnectionSettings(void);
    void reset(void);

};

#endif

