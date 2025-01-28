#include "Provisioning.h"

Provisioning::Provisioning() :
  mqttClient([](const char topic[], byte* payload, unsigned int length) {

  }),
  isRegistered(false) { }

bool Provisioning::addDevice(ProvisioningPayload payload) {

  //mqttClient.connect(settings.clientCert.c_str(), settings.privateKey.c_str());

  return true;

}

void Provisioning::onCertificates(const char topic[], byte* payload, unsigned int length) {
    // Handle the callback here
    // Since this is static, you can't access instance variables directly, but you can
    // refer to a specific instance of Provisioning by passing `this` as context elsewhere.
}