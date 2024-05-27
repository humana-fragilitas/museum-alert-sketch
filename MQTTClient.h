#include <Arduino.h>

// C99 libraries
#include <cstdlib>
#include <string.h>
#include <time.h>

// Libraries for MQTT client and WiFi connection
#include <mqtt_client.h>

// Azure IoT SDK for C includes
#include <az_core.h>
#include <az_iot.h>

#ifndef MQTT_CLIENT
#define MQTT_CLIENT

#define IOT_CONFIG_DEVICE_CERT "-----BEGIN CERTIFICATE-----\r\n" \
"MIIDVzCCAj+gAwIBAgIBATANBgkqhkiG9w0BAQsFADBvMRkwFwYDVQQDExBNQVMt\r\n" \
"RUMzNTdBMTg4NTM0MQswCQYDVQQGEwJJVDEPMA0GA1UECBMGSXRhbGlhMQ8wDQYD\r\n" \
"VQQHEwZUb3Jpbm8xFTATBgNVBAoTDE11c2V1bSBBbGVydDEMMAoGA1UECxMDUiZE\r\n" \
"MB4XDTI0MDUyNTE0MjAxNVoXDTI1MDUyNTE0MjAxNVowbzEZMBcGA1UEAxMQTUFT\r\n" \
"LUVDMzU3QTE4ODUzNDELMAkGA1UEBhMCSVQxDzANBgNVBAgTBkl0YWxpYTEPMA0G\r\n" \
"A1UEBxMGVG9yaW5vMRUwEwYDVQQKEwxNdXNldW0gQWxlcnQxDDAKBgNVBAsTA1Im\r\n" \
"RDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANy+35mTIh9lB1n6AMQ9\r\n" \
"hv1z06fbTIN6B0V3/WdHxEn0+ksrmBZcgaAtLOT1lLnWCzzU4Dw5W9V4SgD8GSJo\r\n" \
"dPMJSXMO+/6IzfYO/cIQTJ1yaI73vqRQJlnWbBr3F8+4zt28W0I3rfrwFllq4nK1\r\n" \
"1pTCpVC47fDcKjvnKqwceQyylfVvEno2HnIiYpJ7jU7W1xM8bBFeMjP4AabFQSSC\r\n" \
"jjlrTVml6q3rnkVdtlEH/axGy9Qeu5jpQgySKW4vg3aNl0aQelWHQSLhWJ4cAlEz\r\n" \
"YemxW4bAXf6pJwJC7CxvCv5+XG10/BaiGUF0iloZEo6CN/3bldqehArzJRvS0xaa\r\n" \
"dscCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEARPddJvnDkugskar3/6hWKnKv7o4I\r\n" \
"eFi1PpXYHqG6FjI3M8Ps5sLrV20pSs3ARv3B04CXwXC7lsMfwALD6sGb/zd/v+GY\r\n" \
"euzRyTpehYoCg8oC4GO4AGa9xmgfy0vdNMhhtlkaQenVz6YqpuqI7l41vSkmt0ol\r\n" \
"HgfmVyXFje0b/QpU1nPd8hr0QGFLN1WKe+bU93QJhmrzVggZjpF6VJzkiN5KieFY\r\n" \
"tpfaff7CS7IyuCtAJbLvERPzYuYSaPAuFOa0Q5/nfopjDhxGKDQH+BTh55KsOGzB\r\n" \
"3QmBOZZJEzsgi158tVKLLfv54enNvSTxlHrTim4puoM5t1Qetj30a4BGJw==\r\n" \
"-----END CERTIFICATE-----\r\n";

#define IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\r\n" \
"MIIEpAIBAAKCAQEA3L7fmZMiH2UHWfoAxD2G/XPTp9tMg3oHRXf9Z0fESfT6SyuY\r\n" \
"FlyBoC0s5PWUudYLPNTgPDlb1XhKAPwZImh08wlJcw77/ojN9g79whBMnXJojve+\r\n" \
"pFAmWdZsGvcXz7jO3bxbQjet+vAWWWricrXWlMKlULjt8NwqO+cqrBx5DLKV9W8S\r\n" \
"ejYeciJiknuNTtbXEzxsEV4yM/gBpsVBJIKOOWtNWaXqreueRV22UQf9rEbL1B67\r\n" \
"mOlCDJIpbi+Ddo2XRpB6VYdBIuFYnhwCUTNh6bFbhsBd/qknAkLsLG8K/n5cbXT8\r\n" \
"FqIZQXSKWhkSjoI3/duV2p6ECvMlG9LTFpp2xwIDAQABAoIBAQCcmpVLAITuvN/Q\r\n" \
"R3qPvg6sdKWtqfjINaQ+9ndB9DofNbrz5UOKaapUlngJHuiaRm3GhEdoslCiSypF\r\n" \
"NJQoQu7lFKuVAwZnd2qWq9/+801HTck71Crdqzbp+SLMpouwBC5ORLiBa7r0Eavp\r\n" \
"V9i7BKHs+4IImInFnIwh30f4vmJqTF6V6Aq0lknl6wbUBnxCOPweXrwAc+Ju77bp\r\n" \
"SsPbuR/gpIDxGR9Q2Q2t0zC2HzzNcJOQR+/i8PX3M1kF9LmYKyflBuvE9E9Sv8Hw\r\n" \
"tjw04kpuW3UZ6l58lAeHRGlr43LVeyAApi8o5PleCRRNHXpG2wAIDA/rHvNib/qo\r\n" \
"7EvQN0LRAoGBAPJwGGGQOAp06b9mEH5QxazlCbA7xO/2Cc/3hwLiITPrg9KHPMzb\r\n" \
"grYFX7TWOIb23RsmJFlxINmGS09zI4nnXYZm8TyDH8yJQMxXPLF4qhDMZAk/+/Ux\r\n" \
"3EFTVJZBFISihNbaymXR5JgdLmqLcwDXPYFEkewXyvl2Dq6Kbz0lmdTjAoGBAOkY\r\n" \
"IKECTUk1uDhnmfU1hHt8pOq6itmWtbPU9ixW/Vp4hOxQyTTZOWt6KiwzIeghW4uF\r\n" \
"EhHWuoRXIy6MBf9psrxrj2xyuAqAxEdWiqvSf71y2zG3hhbaa2q22V1Z1X5qFN4q\r\n" \
"xsd52eoX2YgGfh7RjCUU/yIHjGwD4zwZbZpMpp/NAoGAS3rC3H0+NWM48zIfqHQV\r\n" \
"V0LnxUqWge8kFu+FxUwJ8lQ88mrQbydYhrsdlPutFbf+FtnFL2OdSpwZDl9WjTTP\r\n" \
"VWzvZlucpt2Eoxn193sN17UK4CZfl3Myk9QR3cXdUX4XxZzQruquNP3A2cMYxwY8\r\n" \
"S+bBV7QAqbIr2AOZpnvybOkCgYBws1i7YIiLuCyNIRJga/LVXgvC7lTKJcNO4s3v\r\n" \
"3FN9Fb53IAxYwBqyK4wOeN6RBOflSn7VHzRpXlRFYjBYMPvZfEwJTGJNubqtH1vG\r\n" \
"/e0DZXAz1p8/l3XOUABC0XeXOqVCUf5wXisNs2BbE4CRWBHhsAg3pNyxMSQCX+0N\r\n" \
"aLg7lQKBgQCeU8cNJ/7T7bn/THlksT03nCbmE8topgWgN5A2MwFItosmA9e2XXAk\r\n" \
"LMmYdkbIj/XtPHDcpS9b0Un0YHftlMGgqBgFW5lXSztIKkgOeLKkgLu3XNM651Sq\r\n" \
"t84TZTtOPMkPYROFvJnI4KgpT5er1i/s0Q7fAUiuJKffVQRK8F6oqA==\r\n" \
"-----END RSA PRIVATE KEY-----\r\n";

static char *ROOT = "-----BEGIN CERTIFICATE-----\r\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n" \
"-----END CERTIFICATE-----\r\n";

#define IOT_CONFIG_IOTHUB_FQDN "museum-alert-iot-hub.azure-devices.net"
#define IOT_CONFIG_DEVICE_ID "MAS-EC357A188534"
#define AZ_IOT_DEFAULT_MQTT_CONNECT_PORT 8883
#define INCOMING_DATA_BUFFER_SIZE 128
#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp32)"

// Utility macros and defines
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_QOS1 1
#define DO_NOT_RETAIN_MSG 0
#define UNIX_TIME_NOV_13_2017 1510592825

#define PST_TIME_ZONE -8
#define PST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF 1

#define GMT_OFFSET_SECS (PST_TIME_ZONE * 3600)
#define GMT_OFFSET_SECS_DST ((PST_TIME_ZONE + PST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF) * 3600)

// constexpr inline static int my_constant_member = 42;

class MQTTClient {

  private:

    const char* host = IOT_CONFIG_IOTHUB_FQDN;
    const char* mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
    const char* device_id = IOT_CONFIG_DEVICE_ID;
    const int mqtt_port = AZ_IOT_DEFAULT_MQTT_CONNECT_PORT;
    esp_err_t(*_onMqttEvent)(esp_mqtt_event_handle_t);

    esp_mqtt_client_handle_t mqtt_client;
    az_iot_hub_client client;
    char mqtt_client_id[128];
    char mqtt_username[128];
    char mqtt_password[200];
    uint8_t sas_signature_buffer[256];
    unsigned long next_telemetry_send_time_ms;
    char telemetry_topic[128];
    uint32_t telemetry_send_count;
    //String telemetry_payload;
    char incoming_data[INCOMING_DATA_BUFFER_SIZE];

    std::pair<esp_mqtt_client_handle_t, int> initializeMqttClient();

  public:
    MQTTClient(esp_err_t(*onMqttEvent)(esp_mqtt_event_handle_t));
    std::pair<esp_mqtt_client_handle_t, int> connect();

};

#endif