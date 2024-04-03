//
// Created by Gabriel on 3/23/2024.
//

#ifndef CLOUD_PROV_H
#define CLOUD_PROV_H

#include <core_mqtt.h>

MQTTStatus_t CloudProv_ConnectDevice(MQTTContext_t *mqttContext, MQTTEventCallback_t mqttCallback);


#endif //CLOUD_PROV_H
