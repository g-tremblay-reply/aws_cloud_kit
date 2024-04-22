/***********************************************************************************************************************
 * File Name    : cloud_app.h
 * Description  : Contains functions used in Renesas Cloud Connectivity application
 **********************************************************************************************************************/
#ifndef CLOUD_APP_H
#define CLOUD_APP_H

#include <core_mqtt.h>
#include <console.h>



/**
 * @brief Sensor data types the cloud app can have requested by AWS server
 */
typedef enum
{
    CLOUD_APP_NO_DATA =         (uint8_t)0u,
    CLOUD_APP_IAQ_DATA =        (uint8_t)1u,
    CLOUD_APP_OAQ_DATA =        (uint8_t)2u,
    CLOUD_APP_HS3001_DATA =     (uint8_t)3u,
    CLOUD_APP_ICM_DATA =        (uint8_t)4u,
    CLOUD_APP_ICP_DATA =        (uint8_t)5u,
    CLOUD_APP_OB1203_DATA =     (uint8_t)6u,
    CLOUD_APP_BULK_SENS_DATA =  (uint8_t)7u,
}CloudApp_SensorData_t;

void CloudApp_Init(MQTTContext_t *mqttContext);

void CloudApp_MqttCallback( MQTTContext_t * pMqttContext,
                            MQTTPacketInfo_t * pPacketInfo,
                            MQTTDeserializedInfo_t * pDeserializedInfo );
void CloudApp_MainFunction(MQTTContext_t *mqttContext);


#endif //CLOUD_APP_H
