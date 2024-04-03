/***********************************************************************************************************************
 * File Name    : cloud_app.c
 * Description  : Contains functions used in Renesas Cloud Connectivity application
 **********************************************************************************************************************/

#include <cloud_app.h>
#include <common_data.h>
#include <usr_hal.h>
#include <console.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_DHCP.h>
#include <mqtt_subscription_manager.h>
#include <sensor_ob1203.h>
#include <sensor_iaq.h>
#include <sensor_oaq.h>
#include <sensor_hs3001.h>
#include <sensor_icp10101.h>
#include <ICM_20948.h>

#define CLOUD_APP_PUSH_DATA_PERIOD_SEC  (10u)

#define CLOUD_APP_ETH_CONFIG    "\r\n\r\n--------------------------------------------------------------------------------"\
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
const char *CloudAppPubTopicsNames[CLOUD_APP_PUB_TOPIC_COUNT] =
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

IPV4Parameters_t xNd = {RESET_VALUE,
                        RESET_VALUE,
                        RESET_VALUE,
                        {RESET_VALUE,RESET_VALUE},
                        RESET_VALUE,
                        RESET_VALUE};
static uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] =        { RESET_VALUE };
static uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] =          { 255, 255, 255, 255 };
static uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] =   { RESET_VALUE };
static uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] = {75, 75, 75, 75};
static uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] =       { 0x00, 0x11, 0x22, 0x33, 0x44, 0x57 };
static CloudApp_SensorData_t CloudAppDataRequest = CLOUD_APP_NO_DATA;
static CloudApp_SensorData_t CloudAppDataPush = CLOUD_APP_IAQ_DATA;
static char CloudAppPayloadBuffer[CLOUD_APP_PAYLOAD_BUFFER_SIZE] = {0u};


/**********************************************************************************************************************
                                    LOCAL FUNCTION PROTOTYPES
**********************************************************************************************************************/
/**
 * @brief      Creates and prints  the IP configuration to display on the  console
 * @param[in]  void
 * @retval     None
 */
static void CloudApp_PrintIPConfig(void);

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
    APP_INFO_PRINT( ( "Invoked led callback.\r\n" ) );
    APP_INFO_PRINT("Incoming QOS : %d.\r\n", pPublishInfo->qos);
    APP_INFO_PRINT("Retain flag : %d.\r\n", pPublishInfo->retain);
    strncpy(strlog, pPublishInfo->pPayload, pPublishInfo->payloadLength);
    APP_INFO_PRINT("Incoming Publish Topic matches subscribed topic.\r\n"
               "Incoming Publish Message : %s.\r\n", strlog);

    /* BLUE LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_cold_led_on))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_OFF);
        APP_INFO_PRINT("\r\nCOLD LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("\r\nCOLD LED OFF \r\n");
    }

    /* GREEN LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_warm_led_on))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_OFF);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("\r\nWARM LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);

        APP_INFO_PRINT("\r\nWARM LED OFF \r\n");
    }

    /* RED LED */
    if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_hot_led_on))
    {
        led_on_off (RGB_LED_RED, LED_OFF);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("\r\nHOT LED ON\r\n");
    }
    else if (0u == strcmp ((char *) pPublishInfo->pPayload, msg_temperature_led_off))
    {
        led_on_off (RGB_LED_RED, LED_ON);
        led_on_off (RGB_LED_GREEN, LED_ON);
        led_on_off (RGB_LED_BLUE, LED_ON);
        APP_INFO_PRINT("\r\nHOT LED OFF \r\n");
    }
}


static void CloudApp_Spo2LedCallback(MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo )
{

    const char msg_spo2_led_on[] = "{\"Spo_LED\": \"ON\"}";
    const char msg_spo2_led_off[] = "{\"Spo_LED\": \"OFF\"}";
    char strlog[64] = {'\0'};

    /* Print information about the incoming PUBLISH message. */
    APP_INFO_PRINT("Incoming QOS : %d.\r\n", pPublishInfo->qos);
    APP_INFO_PRINT("Retain flag : %d.\r\n", pPublishInfo->retain);

    strncpy(strlog, pPublishInfo->pPayload, pPublishInfo->payloadLength);
    APP_INFO_PRINT("Incoming Publish Topic matches subscribed topic.\r\n"
    "Incoming Publish Message : %s.\r\n", strlog);

    /* BLUE LED */
    if (RESET_VALUE == strcmp ((char *) pPublishInfo->pPayload, msg_spo2_led_on))
    {
        led_on_off (LED_BLUE, LED_ON);
        APP_INFO_PRINT("\r\nSPO2 LED ON\r\n");
    }
    else if (RESET_VALUE == strcmp ((char *) pPublishInfo->pPayload, msg_spo2_led_off))
    {
        led_on_off (LED_BLUE, LED_OFF);
        APP_INFO_PRINT("\r\nSPO2 LED OFF \r\n");
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


static void CloudApp_PrintIPConfig(void)
{
#if( ipconfigUSE_DHCP != 0 )

    ucNetMask[3] = (uint8_t) ((xNd.ulNetMask & 0xFF000000) >> 24);
    ucNetMask[2] = (uint8_t) ((xNd.ulNetMask & 0x00FF0000) >> 16);
    ucNetMask[1] = (uint8_t) ((xNd.ulNetMask & 0x0000FF00) >> 8);
    ucNetMask[0] = (uint8_t) (xNd.ulNetMask & 0x000000FF);

    ucGatewayAddress[3] = (uint8_t) ((xNd.ulGatewayAddress & 0xFF000000) >> 24);
    ucGatewayAddress[2] = (uint8_t) ((xNd.ulGatewayAddress & 0x00FF0000) >> 16);
    ucGatewayAddress[1] = (uint8_t) ((xNd.ulGatewayAddress & 0x0000FF00) >> 8);
    ucGatewayAddress[0] = (uint8_t) (xNd.ulGatewayAddress & 0x000000FF);

    ucDNSServerAddress[3] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0xFF000000)>> 24);
    ucDNSServerAddress[2] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0x00FF0000)>> 16);
    ucDNSServerAddress[1] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0x0000FF00)>> 8);
    ucDNSServerAddress[0] = (uint8_t)(xNd.ulDNSServerAddresses[0] & 0x000000FF);

    ucIPAddress[3] = (uint8_t)((xNd.ulIPAddress & 0xFF000000) >> 24);
    ucIPAddress[2] = (uint8_t)((xNd.ulIPAddress & 0x00FF0000) >> 16);
    ucIPAddress[1] = (uint8_t)((xNd.ulIPAddress & 0x0000FF00) >> 8);
    ucIPAddress[0] = (uint8_t)(xNd.ulIPAddress & 0x000000FF);
#endif
    APP_PRINT(CLOUD_APP_ETH_CONFIG);

    APP_PRINT("\tDescription . . . . . . . . . . . : Renesas "KIT_NAME" Ethernet\r\n");
    APP_PRINT("\tPhysical Address. . . . . . . . . : %02x-%02x-%02x-%02x-%02x-%02x\r\n", ucMACAddress[0],
              ucMACAddress[1], ucMACAddress[2], ucMACAddress[3], ucMACAddress[4], ucMACAddress[5]);
    APP_PRINT("\tDHCP Enabled. . . . . . . . . . . : %s\r\n", "Yes" );
    APP_PRINT("\tIPv4 Address. . . . . . . . . . . : %d.%d.%d.%d\r\n", ucIPAddress[0], ucIPAddress[1], ucIPAddress[2],
              ucIPAddress[3]);
    APP_PRINT("\tSubnet Mask . . . . . . . . . . . : %d.%d.%d.%d\r\n", ucNetMask[0], ucNetMask[1], ucNetMask[2],
              ucNetMask[3]);
    APP_PRINT("\tDefault Gateway . . . . . . . . . : %d.%d.%d.%d\r\n", ucGatewayAddress[0], ucGatewayAddress[1],
              ucGatewayAddress[2], ucGatewayAddress[3]);
    APP_PRINT("\tDNS Servers . . . . . . . . . . . : %d.%d.%d.%d\r\n", ucDNSServerAddress[0], ucDNSServerAddress[1],
              ucDNSServerAddress[2], ucDNSServerAddress[3]);
}
/**********************************************************************************************************************
                                    GLOBAL FUNCTION PROTOTYPES
**********************************************************************************************************************/
#if( ipconfigUSE_DHCP != 0 )
/**
 * @brief      This is the User Hook for the DHCP Response. xApplicationDHCPHook() is called by DHCP Client Code when DHCP
 *             handshake messages are exchanged from the Server.
 * @param[in]  Different Phases of DHCP Phases and the Offered IP Address
 * @retval     Returns DHCP Answers.
 */
eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t lulIPAddress)
{
    eDHCPCallbackAnswer_t eReturn = eDHCPContinue;
    /*
     * This hook is called in a couple of places during the DHCP process, as identified by the eDHCPPhase parameter.
     */
    switch (eDHCPPhase)
    {
        case eDHCPPhasePreDiscover:
            /*
             *  A DHCP discovery is about to be sent out.  eDHCPContinue is returned to allow the discovery to go out.
             *  If eDHCPUseDefaults had been returned instead then the DHCP process would be stopped and the statically
             *  configured IP address would be used.
             *  If eDHCPStopNoChanges had been returned instead then the DHCP process would be stopped and whatever the
             *  current network configuration was would continue to be used.
             */
            break;

        case eDHCPPhasePreRequest:
            /* An offer has been received from the DHCP server, and the offered IP address is passed in the lulIPAddress
             * parameter.
             */
            /*
             * The sub-domains don’t match, so continue with the DHCP process so the offered IP address is used.
             */
            /* Update the Structure, the DHCP state Machine is not updating this */
            xNd.ulIPAddress = lulIPAddress;
            xNd.ulNetMask = FreeRTOS_GetNetmask();
            xNd.ulGatewayAddress = FreeRTOS_GetGatewayAddress();
            xNd.ulDNSServerAddresses[0] = FreeRTOS_GetDNSServerAddress();
            break;

        default:
            /*
             * Cannot be reached, but set eReturn to prevent compiler warnings where compilers are disposed to generating one.
             */
            break;
    }
    return eReturn;
}
#endif

#if ( ipconfigUSE_NETWORK_EVENT_HOOK == 1 )
/**
 * @brief      Network event callback. Indicates the Network event. Added here to avoid the build errors
 * @param[in]  None
 * @retval     Hostname
 */
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkUp == eNetworkEvent)
    {
        uint32_t lulIPAddress;
        uint32_t lulNetMask;
        uint32_t lulGatewayAddress;
        uint32_t lulDNSServerAddress;
        int8_t lcBuffer[16];

        /* Signal application the network is UP */
        xTaskNotifyFromISR(cloud_app_thread, eNetworkUp, eSetBits, NULL);

/*  TODO something with this
 * The network is up and configured.  Print out the configuration
         obtained from the DHCP server. *//*
        FreeRTOS_GetAddressConfiguration (&lulIPAddress,
                                          &lulNetMask,
                                          &lulGatewayAddress,
                                          &lulDNSServerAddress);

        *//* Convert the IP address to a string then print it out. *//*
        FreeRTOS_inet_ntoa (lulIPAddress, (char*) lcBuffer);

        *//* Convert the net mask to a string then print it out. *//*
        FreeRTOS_inet_ntoa (lulNetMask, (char*) lcBuffer);

        *//* Convert the IP address of the gateway to a string then print it out. *//*
        FreeRTOS_inet_ntoa (lulGatewayAddress, (char*) lcBuffer);

        *//* Convert the IP address of the DNS server to a string then print it out. *//*
        FreeRTOS_inet_ntoa (lulDNSServerAddress, (char*) lcBuffer);*/
    }
}
#endif

void CloudApp_EnableDataPushTimer(void)
{
    g_timer1.p_api->open (g_timer1.p_ctrl, g_timer1.p_cfg);
    g_timer1.p_api->enable (g_timer1.p_ctrl);
    g_timer1.p_api->start (g_timer1.p_ctrl);
}

void CloudApp_InitIPStack(void)
{
    if(FreeRTOS_IPInit_Multi() != pdTRUE)
    {
        FAILURE_INDICATION;
        APP_ERR_PRINT("User Network Initialization Failed");
        APP_ERR_TRAP(pdFALSE);
    }
    else
    {
        APP_PRINT("Waiting for IP stack link up");
        /* Wait on notification for cloud_app_thread Task. This notification will come from
         * vApplicationIPNetworkEventHook() function, which is a FreeRTOS callback defined by the user. Using
         * this patterns allows to have a synchronous IP stack initialization */
        xTaskNotifyWait(pdFALSE, pdFALSE, NULL, portMAX_DELAY);

        /* Indicate that network is up with a LED on the device */
        NETWORK_CONNECT_INDICATION;

        /* Print IP config on console screen */
        CloudApp_PrintIPConfig();
    }
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
                /* A SUBACK from the broker, containing the server response to our subscription request, has been received.
                 * It contains the status code indicating server approval/rejection for the subscription to the single topic
                 * requested. The SUBACK will be parsed to obtain the status code, and this status code will be stored in global
                 * variable #xTopicFilterContext. */
                xResult = MQTT_GetSubAckStatusCodes( pPacketInfo, &pucPayload, &xSize );

                /* Only a single topic filter is expected for each SUBSCRIBE packet. */
                configASSERT( xSize == 1UL );
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
}

MQTTStatus_t CloudApp_SubscribeTopics(MQTTContext_t *mqttContext)
{
    BaseType_t xReturnStatus = pdFAIL;
    SubscriptionManagerStatus_t managerStatus = 0u;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    MQTTSubscribeInfo_t subscriptionList[ CLOUD_APP_SUB_TOPIC_COUNT ] = {0u};
    uint16_t topicNameLength = 0u;
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


    if( managerStatus == SUBSCRIPTION_MANAGER_SUCCESS )
    {
        APP_INFO_PRINT("Subscribing to Cloud App MQTT topics %s.\r\n");
        mqttStatus = MQTT_Subscribe(mqttContext,
                                    subscriptionList,
                                    CLOUD_APP_SUB_TOPIC_COUNT,
                                    MQTT_GetPacketId( mqttContext ) );
    }

    if(mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to send SUBSCRIBE packet to broker with error = %s.",
                MQTT_Status_strerror(mqttStatus ) ) );
    }
    else
    {
        mqttStatus = MQTT_ProcessLoop( mqttContext);
        if(mqttStatus != MQTTSuccess)
        {
            LogError( ( "Failed to receive SUBACK packet to broker with error = %s.",
                    MQTT_Status_strerror(mqttStatus ) ) );
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
                break;
        }

        /* Update request to push data. CloudApp_MainFunction will process this request */
        CloudAppDataPush = lastDataPushed;
        /* reset timer */
        secondsElapsed = 0u;
    }

}

void CloudApp_MainFunction(MQTTContext_t *mqttContext)
{

    MQTTPublishInfo_t pubInfo = {
            .pPayload = CloudAppPayloadBuffer,
            .qos = MQTTQoS1
    };
    MQTTStatus_t mqttStatus;

    /* Receive any Publish message, which will update CloudAppDataRequest in CallBacks if
     * data is received on request topics */
    MQTT_ProcessLoop(mqttContext);

    switch(CloudAppDataRequest)
    {
        case CLOUD_APP_IAQ_DATA:
        {
            rm_zmod4xxx_iaq_1st_data_t iaqData;
            Sensor_IaqGetData(&iaqData);
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
            float oaqData;
            Sensor_OaqGetData(&oaqData);
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
            float temperature, humidity;
            Sensor_Hs3001GetData(&temperature, &humidity);
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
            float temperature, pressure;
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
            pubInfo.pTopicName = CloudAppPubTopicsNames[6u];
            pubInfo.topicNameLength = strlen(CloudAppPubTopicsNames[6u]);

            break;
        }

        default:
            break;
    }

    if(CloudAppDataRequest != CLOUD_APP_NO_DATA)
    {
        /* Publish requested sensor data . */
        mqttStatus = MQTT_Publish( mqttContext,
                                    &pubInfo,
                                   MQTT_GetPacketId(mqttContext) );
        if( mqttStatus != MQTTSuccess )
        {
            APP_ERR_PRINT("Failed to send PUBLISH packet to broker with error = %s.\r\n",
                        MQTT_Status_strerror( mqttStatus ));
        }
        else
        {
            APP_INFO_PRINT("PUBLISH sent for topic %s to broker.\r\n",
                       pubInfo.pTopicName);

            /* Check if PUBACK is received */
            MQTT_ProcessLoop(mqttContext);
            CloudAppDataRequest = CLOUD_APP_NO_DATA;

        }
    }

    switch(CloudAppDataPush)
    {
        case CLOUD_APP_IAQ_DATA:
            break;
        case CLOUD_APP_OAQ_DATA:
            break;
        case CLOUD_APP_HS3001_DATA:
            break;
        case CLOUD_APP_ICM_DATA:
            break;
        case CLOUD_APP_ICP_DATA:
            break;
        case CLOUD_APP_OB1203_DATA:
            break;
        case CLOUD_APP_BULK_SENS_DATA:
            break;
        default:
            break;
    }
    if(CloudAppDataPush != CLOUD_APP_NO_DATA)
    {
        //TODO publish data
        CloudAppDataPush = CLOUD_APP_NO_DATA;
    }
}