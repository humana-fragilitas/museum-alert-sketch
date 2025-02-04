#include "macros.h"

#ifndef CONFIGURATION
#define CONFIGURATION

namespace MqttEndpoints {
  const String awsIoTCoreEndpoint = "avo0w7o1tlck1-ats.iot.eu-west-1.amazonaws.com"; // TO DO: make IoT Core endpoint configurable at build time
  const String certificatesProvisioningTopic = "$aws/certificates/create/json";
  const String certificatesProvisioningResponseTopic = "$aws/certificates/create/json/accepted";
  const String deviceProvisioningTopic = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json";
  const String deviceProvisioningResponseTopic = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json/accepted";
  const String deviceIncomingCommandsTopic = "arn:aws:iot:eu-west-1:767398097786:topic/%s/sub"; // TO DO: make region and account id configurable at build time
  const String deviceOutgoingDataTopic = "arn:aws:iot:eu-west-1:767398097786:topic/%s/pub";
};

struct WiFiCredentials {
  String ssid;
  String password;
  bool isValid() {
    return !ssid.isEmpty() && !password.isEmpty();
  };
  void clear() {
    ssid = "";
    password = "";
  };
};

struct Certificates {
  String clientCert;
  String privateKey;
  bool isValid() {
    return !clientCert.isEmpty() &&
           !privateKey.isEmpty();
  };
  void clear() {
    clientCert = "";
    privateKey = "";
  };
};

struct ProvisioningPayload {
  String certificateOwnershipToken;
  String thingName;
  String company;
  void clear() {
    certificateOwnershipToken = "";
    thingName = "";
    company = "";
  };
};

struct ProvisioningSettings {
  WiFiCredentials wiFiCredentials;
  Certificates certificates;
  bool isValid() {
    return wiFiCredentials.isValid() &&
           certificates.isValid();
  };
  void clear() {
    wiFiCredentials.clear();
    certificates.clear();
  };
};

#endif

