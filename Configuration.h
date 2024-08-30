#include<Arduino.h>
#include <Preferences.h>

#ifndef CONFIGURATION
#define CONFIGURATION

struct ConnectionSettings {
  //String serverCert;
  String clientCert;
  String privateKey;
  String mqttEndpoint;
};

class Configuration {

  private:
    Preferences preferences;
    ConnectionSettings provisioningSettings;

  public:
    std::pair<ConnectionSettings, bool> getConnectionSettings();
    void setConnectionSettings(ConnectionSettings settings);
    void deleteConnectionSettings(void);
    std::pair<ConnectionSettings, bool> getProvisioningConnectionSettings();
    void setProvisioningConnectionSettings(ConnectionSettings settings);
    void deleteProvisioningConnectionSettings(void);
    void reset(void);

};

#endif

