/***********************************************************************************************************************
 * File Name    : cloud_app.c
 * Description  : Contains functions used in Renesas Cloud Connectivity application
 **********************************************************************************************************************/

#include <cloud_app.h>
#include "led/led.h"
#include <console.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_DHCP.h>
#include <mqtt_subscription_manager.h>
#include <sensor_ob1203.h>
#include <sensor_iaq.h>
#include <sensor_oaq.h>
#include <sensor_hs3001.h>
#include <sensor_icp10101.h>
#include <sensor_icm20948.h>

#define CLOUD_APP_PUSH_DATA_PERIOD_SEC  (10u)

#define CLOUD_PROV_ETH_CONFIG    "\r\n\r\n--------------------------------------------------------------------------------"\
                                "\r\nEthernet adapter Configuration for Renesas "KIT_NAME": Post IP Init       "\
                                "\r\n--------------------------------------------------------------------------------\r\n\r\n"

#define CLOUD_APP_SUB_TOPIC_COUNT             (9)

#define CLOUD_APP_PUB_TOPIC_COUNT             (7)

#define CLOUD_APP_PAYLOAD_FORMAT_IAQ_JSON     "{\r\n"\
                                            "\"IAQ\" : {\r\n"\
                                            "      \"TVOC (mg/m^3)\" :\"%f\",\r\n"\
											"      \"Etoh (ppm)\" :\"%f\",\r\n"\
											"      \"eco2 (ppm)\" :\"%f\"\r\n"\
                                            "     }\r\n"\
                                            "}\r\n"

#define CLOUD_APP_PAYLOAD_FORMAT_OAQ_JSON     "{\r\n"\
                                            "\"OAQ\" : {\r\n"\
                                            "      \"air quality (Index)\" :\"%f\"\r\n"\
                                            "\r\n         }\r\n"\
                                            "}\r\n"

#define CLOUD_APP_PAYLOAD_FORMAT_HS3001_JSON  "{\r\n"\
                                            "\"HS3001\" : {\r\n"\
                                            "      \"Humidity ()\" :\"%f\",\r\n"\
                                            "      \"Temperature (F)\" :\"%f\"\r\n"\
                                            "\r\n         }\r\n"\
                                            "}\r\n"


#define CLOUD_APP_PAYLOAD_FORMAT_ICM_JSON     "{\r\n"\
                                            "\"ICM\" : {\r\n"\
                                            "   \"acc\" : {\r\n"\
                                            "      \"x \" :\"%f\",\r\n"\
                                            "      \"y \" :\"%f\",\r\n"\
                                            "      \"z \" :\"%f\"\r\n"\
                                            "   \r\n      },\r\n"\
                                            "   \"mag\" : {\r\n"\
                                            "      \"x \" :\"%f\",\r\n"\
                                            "      \"y \" :\"%f\",\r\n"\
                                            "      \"z \" :\"%f\"\r\n"\
                                            "   \r\n      },\r\n"\
                                            "   \"gyr\" : {\r\n"\
                                            "      \"x \" :\"%f\",\r\n"\
                                            "      \"y \" :\"%f\",\r\n"\
                                            "      \"z \" :\"%f\"\r\n"\
                                            "   \r\n      }\r\n"\
                                            "\r\n      }\r\n"\
                                            "}\r\n"

#define CLOUD_APP_PAYLOAD_FORMAT_ICP_JSON     "{\r\n"\
                                            "\"ICP\" : {\r\n"\
                                            "      \"Temperature (F)\" :\"%f\",\r\n"\
                                            "      \"Pressure (Pa)\" :\"%f\"\r\n"\
                                            "\r\n         }\r\n"\
                                            "}\r\n"

#define CLOUD_APP_PAYLOAD_FORMAT_OB1203_JSON  "{\r\n"\
                                            "\"OB1203\" : {\r\n"\
                                            "      \"spo2 ()\" :\"%f\",\r\n"\
                                            "      \"Heart Rate ()\" :\"%f\",\r\n"\
                                            "      \"Breath rate ()\" :\"%f\",\r\n"\
                                            "      \"P2P ()\" :\"%f\"\r\n"\
                                            "\r\n         }\r\n"\
                                            "}\r\n"

#define CLOUD_APP_PAYLOAD_FORMAT_BULK_JSON     "{\r\n"\
                                             "\"IAQ\" : {\r\n"\
                                             "      \"TVOC (mg/m^3)\" :\"%f\",\r\n"\
                                             "      \"Etoh (ppm)\" :\"%f\",\r\n"\
                                             "      \"eco2 (ppm)\" :\"%f\"\r\n"\
                                             "          },\r\n"\
                                             "\"OAQ\" : {\r\n"\
	                                         "      \"air quality (Index)\" :\"%f\"\r\n"\
	                                         "          },\r\n"\
	                                         "\"HS3001\" : {\r\n"\
	                                         "      \"Humidity ()\" :\"%f\",\r\n"\
	                                         "      \"Temperature (F)\" :\"%f\"\r\n"\
	                                         "             },\r\n"\
                                             "\"ICM\" : {\r\n"\
                                             "   \"acc\" : {\r\n"\
                                             "      \"x \" :\"%f\",\r\n"\
                                             "      \"y \" :\"%f\",\r\n"\
                                             "      \"z \" :\"%f\"\r\n"\
                                             "             },\r\n"\
                                             "   \"mag\" : {\r\n"\
                                             "      \"x \" :\"%f\",\r\n"\
                                             "      \"y \" :\"%f\",\r\n"\
                                             "      \"z \" :\"%f\"\r\n"\
                                             "             },\r\n"\
                                             "   \"gyr\" : {\r\n"\
                                             "      \"x \" :\"%f\",\r\n"\
                                             "      \"y \" :\"%f\",\r\n"\
                                             "      \"z \" :\"%f\"\r\n"\
                                             "             }\r\n"\
                                             "         },\r\n"\
                                             "\"ICP\" : {\r\n"\
                                             "      \"Temperature (F)\" :\"%f\",\r\n"\
                                             "      \"Pressure (Pa)\" :\"%f\"\r\n"\
                                             "            },\r\n"\
	                                         "\"OB1203\" : {\r\n"\
	                                         "      \"spo2 ()\" :\"%f\",\r\n"\
	                                         "      \"Heart Rate ()\" :\"%f\",\r\n"\
	                                         "      \"Breath rate ()\" :\"%f\",\r\n"\
	                                         "      \"P2P ()\" :\"%f\"\r\n"\
	                                         "              }\r\n"\
                                             "}\r\n"
#define CLOUD_APP_PAYLOAD_BUFFER_SIZE ( sizeof( CLOUD_APP_PAYLOAD_FORMAT_BULK_JSON ) + 130u )

/* Topics used for the Publishing update in this Application Project. */

/**
 * @brief Topics to be published to
 * @details this is used when preparing MQTT publish of sensor data
 */
char *CloudAppPubTopicsNames[CLOUD_APP_PUB_TOPIC_COUNT] =
        {
            "aws/topic/iaq_sensor_data",
            "aws/topic/oaq_sensor_data",
            "aws/topic/hs3001_sensor_data",
            "aws/topic/icm_sensor_data",
            "aws/topic/icp_sensor_data",
            "aws/topic/ob1203_sensor_data",
            "aws/topic/bulk_sensor_data",
        };

/**
 * @brief External reference to task handle of cloud app.
 * @details This is needed to be used as input parameter in xTaskNotifyFromISR(), called in
 *          vApplicationIPNetworkEventHook(), a FreeRTOS_IP user callback implemented by cloud_app module.
 */
extern TaskHandle_t cloud_app_thread;
static CloudApp_SensorData_t CloudAppDataRequest = CLOUD_APP_NO_DATA;
static CloudApp_SensorData_t CloudAppDataPush = CLOUD_APP_IAQ_DATA;
static char CloudAppPayloadBuffer[CLOUD_APP_PAYLOAD_BUFFER_SIZE] = {0u};
static uint8_t CLoudAppSubAckReceived = 0u;

/**********************************************************************************************************************
                                    LOCAL FUNCTION PROTOTYPES
**********************************************************************************************************************/
/**
 * @brief
 * @param pContext
 * @param pPublishInfo
 */
static void CloudApp_IAQCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_OAQCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_HS3001Callback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_ICMCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_ICPCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_OB1203Callback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_BulkDataCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_TempLedCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );
static void CloudApp_Spo2LedCallback( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );

/**********************************************************************************************************************
                                    LOCAL FUNCTIONS
**********************************************************************************************************************/
static void CloudApp_TempLedCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    const char msg_cold_led_on[] = "{\"Temperature_LED\": \"COLD\"}";
    const char msg_warm_led_on[] = "{\"Temperature_LED\": \"WARM\"}";
    const char msg_hot_led_on[] = "{\"Temperature_LED\": \"HOT\"}";
    const char msg_temperature_led_off[] = "{\"Temperature_LED\": \"OFF\"}";
    char strlog[64] = {'\0'};

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pContext;

    /* Print information about the incoming PUBLISH message. */
    strncpy(strlog, pPublishInfo->pPayload, pPublishInfo->payloadLength);
    APP_INFO_PRINT("Incoming Publish Message : %s.\r\n", strlog);

    /* BLUE LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_cold_led_on))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_OFF);
        APP_INFO_PRINT("COLD LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("COLD LED OFF \r\n");
    }

    /* GREEN LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_warm_led_on))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_OFF);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("WARM LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);

        APP_INFO_PRINT("WARM LED OFF \r\n");
    }

    /* RED LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_hot_led_on))
    {
        led_on_off (RGB_LED_RED, LED_OFF);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("HOT LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("HOT LED OFF \r\n");
    }
}


static void CloudApp_Spo2LedCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{

    const char msg_spo2_led_on[] = "{\"Spo_LED\": \"ON\"}";
    const char msg_spo2_led_off[] = "{\"Spo_LED\": \"OFF\"}";
    char strlog[64] = {'\0'};

    /* Print information about the incoming PUBLISH message. */
    strncpy(strlog, pPublishInfo->pPayload, pPublishInfo->payloadLength);
    APP_INFO_PRINT("Incoming Publish Message : %s.\r\n", strlog);

    /* BLUE LED */
    if (RESET_VALUE == strcmp ((char *) pPublishInfo->pPayload, msg_spo2_led_on))
    {
        led_on_off (LED_BLUE, LED_ON);
        APP_INFO_PRINT("SPO2 LED ON\r\n");
    }
    else if (RESET_VALUE == strcmp ((char *) pPublishInfo->pPayload, msg_spo2_led_off))
    {
        led_on_off (LED_BLUE, LED_OFF);
        APP_INFO_PRINT("SPO2 LED OFF \r\n");
    }
}

static void CloudApp_IAQCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_IAQ_DATA;
}

static void CloudApp_OAQCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_OAQ_DATA;
}

static void CloudApp_HS3001Callback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_HS3001_DATA;
}

static void CloudApp_ICMCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_ICM_DATA;
}

static void CloudApp_ICPCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_ICP_DATA;
}

static void CloudApp_OB1203Callback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_OB1203_DATA;
}

static void CloudApp_BulkDataCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{
    CloudAppDataRequest = CLOUD_APP_BULK_SENS_DATA;
}


/**********************************************************************************************************************
                                    GLOBAL FUNCTION PROTOTYPES
**********************************************************************************************************************/

void CloudApp_EnableDataPushTimer(void)
{
    g_timer1.p_api->open (g_timer1.p_ctrl, g_timer1.p_cfg);
    g_timer1.p_api->enable (g_timer1.p_ctrl);
    g_timer1.p_api->start (g_timer1.p_ctrl);
}

void CloudApp_MqttCallback( MQTTContext_t * pMqttContext,
                                MQTTPacketInfo_t * pPacketInfo,
                                MQTTDeserializedInfo_t * pDeserializedInfo )
{
    MQTTStatus_t xResult = MQTTSuccess;
    uint8_t * pucPayload = NULL;
    size_t xSize = 0;

    configASSERT( pMqttContext != NULL );
    configASSERT( pPacketInfo != NULL );
    configASSERT( pDeserializedInfo != NULL );

    /* Indicate on Cloud Kit some MQTT activity by toggling blue led*/
    AWS_ACTIVITY_INDICATION;

    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
    {
        configASSERT( pDeserializedInfo->pPublishInfo != NULL );
        /* Handle incoming publish. */
        SubscriptionManager_DispatchHandler( pMqttContext, pDeserializedInfo->pPublishInfo );
    }
    else
    {
        /* Handle other packets. */
        switch( pPacketInfo->type )
        {
            case MQTT_PACKET_TYPE_SUBACK:
                APP_INFO_PRINT("MQTT_PACKET_TYPE_SUBACK\r\n");
                CLoudAppSubAckReceived ++;
                /* A SUBACK from the broker, containing the server response to our subscription request, has been received.
                 * It contains the status code indicating server approval/rejection for the subscription to the single topic
                 * requested. The SUBACK will be parsed to obtain the status code, and this status code will be stored in global
                 * variable #xTopicFilterContext. */
                xResult = MQTT_GetSubAckStatusCodes( pPacketInfo, &pucPayload, &xSize );

                /* Only a single topic filter is expected for each SUBSCRIBE packet. */
                //configASSERT( xSize == 1UL ); TODO figure out what this byte is. Maybe topic count in sub request?
                uint8_t subackSts;
                subackSts = *pucPayload;

                if( subackSts != MQTTSubAckFailure )
                {
                    APP_INFO_PRINT("Subscribed to the topic %s with maximum QoS %u.\r\n",
                               pDeserializedInfo->pPublishInfo->pTopicName,
                               subackSts);
                    APP_INFO_PRINT("ACK packet identifier %d.\r\n", pDeserializedInfo->packetIdentifier);
                }
                break;

            case MQTT_PACKET_TYPE_UNSUBACK:
                APP_INFO_PRINT("MQTT_PACKET_TYPE_UNSUBACK\r\n" );
                break;

            case MQTT_PACKET_TYPE_PINGRESP:
                /* Nothing to be done from application as library handles
                 * PINGRESP. */
                APP_WARN_PRINT("PINGRESP should not be handled by the application "
                           "callback when using MQTT_ProcessLoop.\r\n" );
                break;

            case MQTT_PACKET_TYPE_PUBACK:
                APP_INFO_PRINT("PUBACK received for packet id %u.\r\n", pDeserializedInfo->packetIdentifier);
                break;

            default:
                /* Any other packet type is invalid. */
                APP_ERR_PRINT("Unknown packet type received:(%02x).\r\n", pPacketInfo->type);
        }
    }
    AWS_ACTIVITY_INDICATION;
}

MQTTStatus_t CloudApp_SubscribeTopics(MQTTContext_t *mqttContext)
{
    SubscriptionManagerStatus_t managerStatus = 0u;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    MQTTSubscribeInfo_t subscriptionList[ CLOUD_APP_SUB_TOPIC_COUNT ] = {0u};
    uint16_t topicNameLength;
    const char *topicsName[CLOUD_APP_SUB_TOPIC_COUNT] =
            {
                    "aws/topic/get_iaq_sensor_data",
                    "aws/topic/get_oaq_sensor_data",
                    "aws/topic/get_hs3001_sensor_data",
                    "aws/topic/get_icm_sensor_data",
                    "aws/topic/get_icp_sensor_data",
                    "aws/topic/get_ob1203_sensor_data",
                    "aws/topic/get_bulk_sensor_data",
                    "aws/topic/set_temperature_led_data",
                    "aws/topic/set_spo2_led_data",
            };
    SubscriptionManagerCallback_t callbacks[CLOUD_APP_SUB_TOPIC_COUNT] =
            {
                    CloudApp_IAQCallback,
                    CloudApp_OAQCallback,
                    CloudApp_HS3001Callback,
                    CloudApp_ICMCallback,
                    CloudApp_ICPCallback,
                    CloudApp_OB1203Callback,
                    CloudApp_BulkDataCallback,
                    CloudApp_TempLedCallback,
                    CloudApp_Spo2LedCallback
            };


    /* Register topicsName and their callback with subscription manager.
     * On an incoming PUBLISH message whose topic name that matches the topic filter
     * being registered, its callback will be invoked. */
    for(uint8_t topic=0u; topic<CLOUD_APP_SUB_TOPIC_COUNT; topic++)
    {
        topicNameLength = strlen(topicsName[topic]);
        managerStatus |= SubscriptionManager_RegisterCallback(topicsName[topic],
                                                              topicNameLength,
                                                              callbacks[topic] );
        /* Populate subscription list with topic info */
        subscriptionList[topic].pTopicFilter = topicsName[topic];
        subscriptionList[topic].topicFilterLength = topicNameLength;
        subscriptionList[topic].qos = MQTTQoS1;
    }

    /* Indicate start of interaction with MQTT. This sets the led to OFF, and sybsequent MQTT
     * receive callbacks/publishes will toggle it further */
    AWS_ACTIVITY_INDICATION;

    if( managerStatus == SUBSCRIPTION_MANAGER_SUCCESS )
    {
        APP_INFO_PRINT("Correctly registered callbacks to CloudApp topics.\r\n");

        /* Send subscribe packts in 2 separate messages because AWS server does not accept all
         * topics in one message */
        mqttStatus = MQTT_Subscribe(mqttContext,
                                    subscriptionList,
                                    5u,
                                    MQTT_GetPacketId( mqttContext ) );
        if(mqttStatus != MQTTSuccess )
        {
            APP_WARN_PRINT( ( "Failed to send SUBSCRIBE packet to broker with error = %s.\r\n"),
                            MQTT_Status_strerror(mqttStatus) );
        }
        mqttStatus = MQTT_Subscribe(mqttContext,
                                    &subscriptionList[5],
                                    4u,
                                    MQTT_GetPacketId( mqttContext ) );
        if(mqttStatus != MQTTSuccess )
        {
            APP_WARN_PRINT( ( "Failed to send SUBSCRIBE packet to broker with error = %s.\r\n"),
                            MQTT_Status_strerror(mqttStatus) );
        }
    }

    if(mqttStatus == MQTTSuccess )
    {
        uint16_t timeMs = 0u;
        /* Check until 2 SUBACK packets are received  */
        while((timeMs <= 5000u) && (CLoudAppSubAckReceived <= 2))
        {
            mqttStatus = MQTT_ProcessLoop( mqttContext);
            timeMs += 1000u;
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
        if(CLoudAppSubAckReceived < 2u)
        {
            APP_WARN_PRINT( ( "Failed to receive SUBACK packets from broker with error = %s.\r\n"),
                            MQTT_Status_strerror(mqttStatus) );
        }
    }
    return mqttStatus;
}

void CloudApp_PeriodicDataPush(timer_callback_args_t *p_args)
{
    (void) (p_args);
    static uint16_t secondsElapsed = 0;
    static CloudApp_SensorData_t lastDataPushed = CLOUD_APP_IAQ_DATA;

    secondsElapsed++;
    /* Check if cloud app data push period is  */
    if (secondsElapsed >= CLOUD_APP_PUSH_DATA_PERIOD_SEC)
    {
        /* Fill latest sensor data into structure */
        switch(lastDataPushed)
        {
            case CLOUD_APP_IAQ_DATA:
                lastDataPushed = CLOUD_APP_OAQ_DATA;
                break;
            case CLOUD_APP_OAQ_DATA:
                lastDataPushed = CLOUD_APP_HS3001_DATA;
                break;
            case CLOUD_APP_HS3001_DATA:
                lastDataPushed = CLOUD_APP_ICM_DATA;
                break;
            case CLOUD_APP_ICM_DATA:
                lastDataPushed = CLOUD_APP_ICP_DATA;
                break;
            case CLOUD_APP_ICP_DATA:
                lastDataPushed = CLOUD_APP_OB1203_DATA;
                break;
            case CLOUD_APP_OB1203_DATA:
                lastDataPushed = CLOUD_APP_IAQ_DATA;
                break;
            default:
                /* If this state is reached, this cloudkit is cursed */
            	lastDataPushed = CLOUD_APP_IAQ_DATA;
                break;
        }

        /* Update request to push data. CloudApp_MainFunction will process this request */
        CloudAppDataPush = lastDataPushed;
        /* reset timer */
        secondsElapsed = 0u;
    }

}

static void CloudApp_PublishSensorData(MQTTContext_t *mqttContext, CloudApp_SensorData_t sensorData)
{
    MQTTStatus_t mqttStatus;
    MQTTPublishInfo_t pubInfo = {
            .pPayload = CloudAppPayloadBuffer,
            .qos = MQTTQoS1
    };

    /* Populate Sensor data publish message */
    switch(sensorData)
    {
        case CLOUD_APP_IAQ_DATA:
        {
            rm_zmod4xxx_iaq_1st_data_t iaqData;
            SensorIaq_GetData(&iaqData);
            pubInfo.pTopicName = CloudAppPubTopicsNames[0u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[0u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_IAQ_JSON,
                                              (double)iaqData.tvoc,
                                              (double)iaqData.etoh,
                                              (double)iaqData.eco2);
            break;
        }
        case CLOUD_APP_OAQ_DATA:
        {
            float_t oaqData;
            SensorOaq_GetData(&oaqData);
            pubInfo.pTopicName = CloudAppPubTopicsNames[1u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[1u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_OAQ_JSON,
                                              (double)oaqData);
            break;
        }
        case CLOUD_APP_HS3001_DATA:
        {
            float_t temperature, humidity;
            SensorHs3001_GetData(&temperature, &humidity);
            pubInfo.pTopicName = CloudAppPubTopicsNames[2u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[2u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_HS3001_JSON,
                                              (double)humidity,
                                              (double)temperature);
            break;
        }
        case CLOUD_APP_ICM_DATA:
        {
            float_t temperature, pressure;
            SensorIcp10101_GetData(&temperature, &pressure);
            pubInfo.pTopicName = CloudAppPubTopicsNames[3u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[3u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_ICP_JSON,
                                              (double)temperature,
                                              (double)pressure);
            break;
        }
        case CLOUD_APP_ICP_DATA:
        {
            xyzFloat acc, gyr, magnitude;
            SensorIcm20948_GetData(&acc, &gyr, &magnitude);
            pubInfo.pTopicName = CloudAppPubTopicsNames[4u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[4u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_ICM_JSON,
                                              acc.x,
                                              acc.y,
                                              acc.z,
                                              magnitude.x,
                                              magnitude.y,
                                              magnitude.z,
                                              gyr.x,
                                              gyr.y,
                                              gyr.z);
            break;
        }
        case CLOUD_APP_OB1203_DATA:
        {
            ob1203_bio_data_t ob1203data;
            Sensor_Ob1203GetData(&ob1203data);
            pubInfo.pTopicName = CloudAppPubTopicsNames[5u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[5u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_OB1203_JSON,
                                              (double)ob1203data.spo2,
                                              (double)ob1203data.heart_rate,
                                              (double)ob1203data.respiration_rate,
                                              (double)ob1203data.perfusion_index);
            break;

        }
        case CLOUD_APP_BULK_SENS_DATA:
        {
            rm_zmod4xxx_iaq_1st_data_t iaqData;
            float_t oaqData;
            float_t tempHs3001, humidity;
            float_t tempIcp, pressure;
            xyzFloat acc, gyr, magnitude;
            ob1203_bio_data_t ob1203data;

            SensorIaq_GetData(&iaqData);
            SensorOaq_GetData(&oaqData);
            SensorHs3001_GetData(&tempHs3001, &humidity);
            SensorIcp10101_GetData(&tempIcp, &pressure);
            SensorIcm20948_GetData(&acc, &gyr, &magnitude);
            Sensor_Ob1203GetData(&ob1203data);

            pubInfo.pTopicName = CloudAppPubTopicsNames[6u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[6u]);
            /* Populate payload buffer and paayload length according to data format */
            pubInfo.payloadLength = snprintf (CloudAppPayloadBuffer,
                                              sizeof(CloudAppPayloadBuffer),
                                              CLOUD_APP_PAYLOAD_FORMAT_BULK_JSON,
                                                (double)iaqData.tvoc,
                                                (double)iaqData.etoh,
                                                (double)iaqData.eco2,
                                                (double)oaqData,
                                                (double)humidity,
                                                (double)tempHs3001,
                                                acc.x,
                                                acc.y,
                                                acc.z,
                                                magnitude.x,
                                                magnitude.y,
                                                magnitude.z,
                                                gyr.x,
                                                gyr.y,
                                                gyr.z,
                                                (double)tempIcp,
                                                (double)pressure,
                                                (double)ob1203data.spo2,
                                                (double)ob1203data.heart_rate,
                                                (double)ob1203data.respiration_rate,
                                                (double)ob1203data.perfusion_index);
            break;
        }

        default:
            break;
    }

    /* Check if MQTT is correctly init, then publish requested sensor data . */
    if(mqttContext->transportInterface.send != NULL)
    {
        /* Toggle CLoudKit lit to indicate MQTT activity */
        AWS_ACTIVITY_INDICATION;
        mqttStatus = MQTT_Publish( mqttContext,
                                   &pubInfo,
                                   MQTT_GetPacketId(mqttContext) );
        AWS_ACTIVITY_INDICATION;
        if( mqttStatus == MQTTSuccess )
        {
            /* Check if PUBACK is received */
            MQTT_ProcessLoop(mqttContext);

        }
        else
        {
            APP_ERR_PRINT("Failed to publish CloudApp Sensor Data with error status = %s.\r\n",
                          MQTT_Status_strerror( mqttStatus ));
        }
    }


    APP_INFO_PRINT(("Published CloudApp sensor data %.*s\r\n"),
                       pubInfo.payloadLength,
                       pubInfo.pPayload);

}

void CloudApp_MainFunction(MQTTContext_t *mqttContext)
{
    /* Check if mqtt context is correctly init, then receive any Publish message,  which will
     * update CloudAppDataRequest in CallBacks if data is received on request topics */
    if(mqttContext->transportInterface.recv != NULL)
    {
        MQTT_ProcessLoop(mqttContext);
    }

    /* Process data requested by MQTT broker */
    if(CloudAppDataRequest != CLOUD_APP_NO_DATA)
    {
        CloudApp_PublishSensorData(mqttContext, CloudAppDataRequest);
        CloudAppDataRequest = CLOUD_APP_NO_DATA;
    }

    /* Process data pushed cyclically by CloudApp */
    if(CloudAppDataPush != CLOUD_APP_NO_DATA)
    {
        CloudApp_PublishSensorData(mqttContext, CloudAppDataPush);

        CloudAppDataPush = CLOUD_APP_NO_DATA;
    }
}
