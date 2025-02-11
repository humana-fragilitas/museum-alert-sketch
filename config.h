#ifndef CONFIG
#define CONFIG

#define CONF_AWS_REGION "eu-west-1"
#define CONF_AWS_IOT_CORE_ACCOUNT_IDENTIFIER "avo0w7o1tlck1"
#define CONF_AWS_ACCOUNT_ID "767398097786"

// Note: AWS IoT Core endpoint is structure as follows: <AWS IoT Core account identifier>-ats.iot.<AWS region>.amazonaws.com;
// the -ats suffix indicates that AWS IoT Core uses Amazon Trust Services (ATS) certificates for authentication.
#define CONF_AWS_IOT_CORE_ENDPOINT CONF_AWS_IOT_CORE_ACCOUNT_IDENTIFIER "-ats.iot." CONF_AWS_REGION ".amazonaws.com"
#define CONF_AWS_CERTIFICATES_PROVISIONING_TOPIC "$aws/certificates/create/json"
#define CONF_AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC "$aws/certificates/create/json/accepted"
#define CONF_AWS_DEVICE_PROVISIONING_TEMPLATE "museum-alert-provisioning-template"
#define CONF_AWS_DEVICE_PROVISIONING_TOPIC "$aws/provisioning-templates/" CONF_AWS_DEVICE_PROVISIONING_TEMPLATE "/provision/json"
#define CONF_AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC "$aws/provisioning-templates/" CONF_AWS_DEVICE_PROVISIONING_TEMPLATE "/provision/json/accepted"

#define CONF_DEVICE_INCOMING_COMMANDS_TOPIC_TEMPLATE "arn:aws:iot:" CONF_AWS_REGION ":" CONF_AWS_ACCOUNT_ID ":topic/%s/sub"
#define CONF_DEVICE_OUTGOING_DATA_TOPIC_TEMPLATE "arn:aws:iot:" CONF_AWS_REGION ":" CONF_AWS_ACCOUNT_ID ":topic/%s/pub"

#endif


