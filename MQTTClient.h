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
             "MIIDSzCCAjOgAwIBAgIBATANBgkqhkiG9w0BAQUFADBpMRQwEgYDVQQDEwtleGFt\r\n" \
             "cGxlLm9yZzELMAkGA1UEBhMCVVMxETAPBgNVBAgTCFZpcmdpbmlhMRMwEQYDVQQH\r\n" \
             "EwpCbGFja3NidXJnMQ0wCwYDVQQKEwRUZXN0MQ0wCwYDVQQLEwRUZXN0MB4XDTI0\r\n" \
             "MDUwNTE2Mjc1MFoXDTI1MDUwNTE2Mjc1MFowaTEUMBIGA1UEAxMLZXhhbXBsZS5v\r\n" \
             "cmcxCzAJBgNVBAYTAlVTMREwDwYDVQQIEwhWaXJnaW5pYTETMBEGA1UEBxMKQmxh\r\n" \
             "Y2tzYnVyZzENMAsGA1UEChMEVGVzdDENMAsGA1UECxMEVGVzdDCCASIwDQYJKoZI\r\n" \
             "hvcNAQEBBQADggEPADCCAQoCggEBAN3Rjx5EMPKbJ3fadAiTTsu4c7m0yTQK9Qqi\r\n" \
             "sNBfmnJVgV+eIaMlFehIYb25E5ltoYoJDRqFFfHYNukY2Vp0uAd6qph+yKthpEfw\r\n" \
             "lpvPegIc8G/kWnO+qo/2k4NOvyr2/piN1f8XfRhG+DsMl2hc9l9NVGLakVxliYtV\r\n" \
             "czWltYchpHdFlD6tiNbuAalSEDyOVo5lDLSPd2Kbz6GA+j5MvFNd7CQQYGyXnL8G\r\n" \
             "lhBg/2Qs5B76mWPDSQ4TypOi7mpUZhByWFX69Eil46olPV9jaIwUTposB0GQivbD\r\n" \
             "lgzHYUOEOpA/qVLvwQaO2bu1jHUdgztNgK+lx4Cq/kqhAV5ustUCAwEAATANBgkq\r\n" \
             "hkiG9w0BAQUFAAOCAQEAPCvvAwLR1ZF7Iz3XdtC/SyMIexUqTkeHHan+EzgbIW8m\r\n" \
             "Q5PgjLalpiJChxFBINO4t4iPTPxKi5q0v4a69NoHK03YDWAKZ3XRlIhZ51ZuBdZf\r\n" \
             "0Lh3YL/luvC1Z3eexBs15m4fcjnOm2xJ9ZG+ej8k0KVSZSA5GVUHdqT0S+JIpiLQ\r\n" \
             "7MGQztOKzzCOk8siXyvT80BoAlQgjtczNMINCvW01Mq3kS/HXZIoS6Thhpz2CiKu\r\n" \
             "CJaY5qt9ftK7UPfdHyAzUdB4E17A2CmSwt5Hk7DkxHiubrOL6rVBBZNqlQsXGhbn\r\n" \
             "sJPRQ0RKsydyb+0HJEK4A7bSqEzMtL8lQH6obIoo6A==\r\n" \
             "-----END CERTIFICATE-----\r\n";

#define IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\r\n" \
            "MIIEpAIBAAKCAQEA3dGPHkQw8psnd9p0CJNOy7hzubTJNAr1CqKw0F+aclWBX54h\r\n" \
            "oyUV6EhhvbkTmW2higkNGoUV8dg26RjZWnS4B3qqmH7Iq2GkR/CWm896Ahzwb+Ra\r\n" \
            "c76qj/aTg06/Kvb+mI3V/xd9GEb4OwyXaFz2X01UYtqRXGWJi1VzNaW1hyGkd0WU\r\n" \
            "Pq2I1u4BqVIQPI5WjmUMtI93YpvPoYD6Pky8U13sJBBgbJecvwaWEGD/ZCzkHvqZ\r\n" \
            "Y8NJDhPKk6LualRmEHJYVfr0SKXjqiU9X2NojBROmiwHQZCK9sOWDMdhQ4Q6kD+p\r\n" \
            "Uu/BBo7Zu7WMdR2DO02Ar6XHgKr+SqEBXm6y1QIDAQABAoIBAHsEj3TfqK3Dsn3b\r\n" \
            "32IqIBcHctbZFoUQVpnRZHILs2IZXaij0E/kb2PlUJ+hlucOT/p3zpaYnHUFzl4z\r\n" \
            "88cg2Db9psLv/WZevndPTJeY1zd4yTek0y1B3uH8hA7ci1TOqp/8eWQBqNTf9yb4\r\n" \
            "crfkZpawEk7InLk7bq6hc0WbYzNPQs+WdRyqMDy6rQU7tz5/9wXm0442V1l0qUAH\r\n" \
            "U5qoNKOGIxOt9ODmbKRvZfu/Ti/LzoX+R6bXUvzNmASgRJvMoZmXI71ltp+dp4QO\r\n" \
            "bBz74JV7VQglW+VuTM5ZgsjLlxAidAzR7PiDuQRUKwi6xrPBZFe81Kt5z+UEoqyu\r\n" \
            "steyj7kCgYEA7udOB80Y6v9PrrirFeE/nOjipkA18tE3vixXyfc2mQAUqrezOtsJ\r\n" \
            "0LYk1OBDyV+JXLQ5/n0QWe2JkpijpVMP3uPgbXs5swc0Gd3IJtfY6rppKMmZ9G2v\r\n" \
            "4QC4bUwz8hzGpiGXLTuInE9JnYD1Fqau3vD5QmAfsRVmegg5BGI0VKMCgYEA7bFC\r\n" \
            "YxOrJi6nkRPkb3kbU2CG99n5CkuwbY1xSZ641O8b9lBIEaTNKa2K5GzVVygcBoYe\r\n" \
            "D0O8US9FSTigBYtHMdWii2mVzABnU++RKnPRybkvrEqS6yAZnYSSabfzvzHUjdvc\r\n" \
            "BA3ugdC+xFDpZohSRSsIsV6mTv/+IHy3TOlB2icCgYBBQjHQc7fwyVkM0yj4yxTD\r\n" \
            "WeI/o56Y+4mMizRX2Q2y8ZCzqYZt91NRDdA1ziT2JwCwsBJ/b849omNIBiX9jRjy\r\n" \
            "u7Ccd2Klgrw6rJh189QvGkiLebZyTFwUzEuUn5I4+p9Q1jAjjPWeLRJg0c8hJtrx\r\n" \
            "z0VnWjaN+tiJUSGyDtlYnQKBgQCbEquyQhHGLmgKgcTmaUfHpNFJgL821Vy8jKwd\r\n" \
            "kN0bpwhCMexi/ncPuvZDwzGI1FU8eGHCKboB6Wo1tCjKlSyUucF7XR5q4tSG8WRv\r\n" \
            "IvL6vcP5jwm9Nssfdm+jY202DweSqZ8oUgKKVSswn203BLdQAxx/w7WTEva0MUnR\r\n" \
            "dtkMvwKBgQC1SGDAQmNfk5krtsULtCNM+vw3FEuQgl74t9SETuHvhLZLS/H3MTT+\r\n" \
            "avqG/cjz3bBQZU98VMzTlLkzKm4gWegfFm5OMCkuhniW+7AgS3GTlIAfBSt4aVM7\r\n" \
            "34xcrXLK156OJAIsqXRCXVzCFJ4RPjJyfM80fKaa2nOb5kSEYklKeQ==\r\n" \
            "-----END RSA PRIVATE KEY-----\r\n"

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

    void initializeIoTHubClient();
    std::pair<esp_mqtt_client_handle_t, int> initializeMqttClient();

  public:
    MQTTClient(esp_err_t(*onMqttEvent)(esp_mqtt_event_handle_t));
    std::pair<esp_mqtt_client_handle_t, int> connect();

};

#endif