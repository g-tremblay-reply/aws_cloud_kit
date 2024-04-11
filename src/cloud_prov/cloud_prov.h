//
// Created by Gabriel on 3/23/2024.
//

#ifndef CLOUD_PROV_H
#define CLOUD_PROV_H

#include <core_mqtt.h>

MQTTStatus_t CloudProv_ProvisionDevice(MQTTContext_t *mqttContext, MQTTEventCallback_t mqttCallback);
uint8_t CloudProv_ImportMqttEndpoint(uint8_t *endpointBuffer, size_t endpointLength);
uint8_t CloudProv_ImportClaimCertificate(uint8_t *endpointBuffer, size_t endpointLength, bool forceProvisioning);
uint8_t CloudProv_ImportClaimPrivateKey(uint8_t *endpointBuffer, size_t endpointLength, bool forceProvisioning);
void CloudProv_InitIPStack(void);
MQTTStatus_t CloudProv_Init(MQTTContext_t * mqttContext, MQTTEventCallback_t appMqttCallback);

#endif //CLOUD_PROV_H
