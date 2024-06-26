//
// Created by Gabriel on 3/23/2024.
//

#include <cloud_prov.h>
#include <cloud_prov_config.h>
#include <cloud_prov_serializer.h>
#include <cloud_prov_pkcs11.h>
#include <console.h>
#include <led.h>
#include <FreeRTOS_DNS.h>
#include <fleet_provisioning.h>
#include <backoff_algorithm.h>

/*************************************************************************************
 * DEFINE MACROS
 ************************************************************************************/

#define CLOUD_PROV_DEVICE_UUID_SIZE_BYTES (16u)

/**
 * @brief Size of buffer in which to hold the certificate signing request (CSR).
 */
#define CLOUD_PROV_CERT_BUFFER_SIZE                   (2048)

/**
 * @brief The length of the outgoing publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for outgoing publishes.
 * This length depends on the Number of publishes & can be updated accordingly.
 */
#define CLOUD_PROV_OUTGOING_PUBLISH_RECORD_LEN       ( 15U )

/**
 * @brief The length of the incoming publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for incoming publishes.
 * This length depends on the Number of publishes & can be updated accordingly.
 */
#define CLOUD_PROV_INCOMING_PUBLISH_RECORD_LEN       ( 15U )

/**
 * @brief number of topics needed in the fleet provisioning workflow
 */
#define CLOUD_PROV_FLEET_PROV_TOPIC_COUNT           (4u)

#define CLOUD_PROV_CERT_ID_BUFFER_SIZE              (64)

/**
 * @brief Size of buffer in which to hold the certificate ownership token.
 */
#define CLOUD_PROV_OWNERSHIP_TOKEN_BUFFER_SIZE      (1024)

/**
 * @brief Size of AWS IoT Thing name buffer.
 *
 * See https://docs.aws.amazon.com/iot/latest/apireference/API_CreateThing.html#iot-CreateThing-request-thingName
 */
#define CLOUD_PROV_THING_NAME_BUFFER_SIZE           (128)
#define CLOUD_PROV_MQTT_ENDPOINT_BUFFER_SIZE        (128)
#define CLOUD_PROV_CLAIM_CERT_BUFFER_SIZE           (2048)
#define CLOUD_PROV_CLAIM_PRIVATE_KEY_BUFFER_SIZE    (2048)


/*************************************************************************************
 * Type Definitions
 ************************************************************************************/
/**
 * @brief Enum allowing to make subscribe/unsubscribe function reusable for fleet prov topics
 */
typedef enum
{
    CloudProv_Subscribe,
    CloudProv_Unsub
}CloudProvTopicAction_t;

/**
 * @brief Each compilation unit that consumes the NetworkContext must define it.
 * It should contain a single pointer to the type of your desired transport.
 * When using multiple transports in the same compilation unit, define this pointer as void *.
 *
 * @note Transport stacks are defined in FreeRTOS-Plus/Source/Application-Protocols/network_transport.
 */
struct NetworkContext
{
    TlsTransportParams_t * pxParams;
};

/*************************************************************************************
 * Local Variables
 ************************************************************************************/
static char cBuffer[16] = { RESET_VALUE };

/** @brief The network context used for mbedTLS operation. */
static CK_SESSION_HANDLE CloudProvP11Session;

/** @brief TLS transport parameters used by mbedTLS operation.
 * @details This struct should be assigned to a NetworkContext_t struct statically */
static TlsTransportParams_t CloudProvTlsTransportParams;

/** @brief The parameters for network context to be handled by FreeRTOS's TLS operations, and integrated
 *          into an MQTT context by the MQTT APIs*/
static NetworkContext_t CloudProvNetworkContext =
        {
            .pxParams = &CloudProvTlsTransportParams
        };

/** @brief Static buffer used internally MQTT library fort messages being sent and received. */
static uint8_t CloudProvMqttSharedBuffer[ CLOUD_PROV_MQTT_BUFFER_SIZE ];

/** @brief Static buffer struct used internally MQTT library fort messages being sent and received */
static MQTTFixedBuffer_t CloudProvMqttBuffer =
        {
                CloudProvMqttSharedBuffer,
                CLOUD_PROV_MQTT_BUFFER_SIZE
        };

/** @brief Copy of publish info invoked in MQTT application callback
 * @details This should be used to access payload of the latest received publish message */
MQTTPublishInfo_t CloudProvPublishInfo;

/** @brief Array to track the outgoing publish records for outgoing publisheswith QoS > 0.
 * @details This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0. */
static MQTTPubAckInfo_t CloudProvOutgoingPublishRecords[ CLOUD_PROV_OUTGOING_PUBLISH_RECORD_LEN ];

/** @brief Array to track the incoming publish records for incoming publishes with QoS > 0.
 * @details This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0. */
static MQTTPubAckInfo_t CloudProvIncomingPublishRecords[ CLOUD_PROV_INCOMING_PUBLISH_RECORD_LEN ];


/**
 * @brief Buffer to hold the provisioned AWS IoT Thing name.
 */
static char CloudProvThingName[ CLOUD_PROV_THING_NAME_BUFFER_SIZE ];

static FleetProvisioningTopic_t CloudProvFleetTopic = FleetProvisioningInvalidTopic;
static bool CLoudProvForceProvisioning = false;
static char CloudProvMqttEndpoint[CLOUD_PROV_MQTT_ENDPOINT_BUFFER_SIZE] = CLOUD_PROV_DEFAULT_MQTT_BROKER_ENDPOINT;
static char CloudProvClaimCert[CLOUD_PROV_CLAIM_CERT_BUFFER_SIZE] = CLOUD_PROV_DEFAULT_CLAIM_CERT_PEM;
static char CloudProvClaimPrivateKey[CLOUD_PROV_CLAIM_PRIVATE_KEY_BUFFER_SIZE] = CLOUD_PROV_DEFAULT_CLAIM_PRIVATE_KEY_PEM;

/*************************************************************************************
 * Local Function Prototypes
 ************************************************************************************/

/**
 * @brief       configures the littleFS Flash for Data storage.
 * @retval      LFS_ERR_OK              If both the connectivity checks are success.
 * @retval      Any other error         If one of the connectivity check fails.
 */
static int CloudProv_InitLittleFs(void);

/**
 * @brief Callback to receive the incoming publish messages from the MQTT broker. Sets xResponseStatus
 * if an expected CreateCertificateFromCsr or RegisterThing response is received, and copies the response into
 * responseBuffer if the response is an accepted one.
 *
 * @param[in] pPublishInfo Pointer to publish info of the incoming publish.
 * @param[in] usPacketIdentifier Packet identifier of the incoming publish.
 */
static void CloudProv_MqttCallback(MQTTContext_t * pxMqttContext,
                                   MQTTPacketInfo_t * pxPacketInfo,
                                   MQTTDeserializedInfo_t * pxDeserializedInfo );

/**
 * @brief Connect to MQTT broker with reconnection retries.
 *
 * @details If connection fails, retry is attempted after a timeout.
 * Timeout value will exponentially increase until maximum
 * timeout value is reached or the number of attempts are exhausted.
 *
 * @param[out] networkContext The output parameter to return the created network context.
 *
 * @return The status of the final connection attempt.
 */
static TlsTransportStatus_t CloudProv_ConnectTLS(NetworkContext_t * networkContext);

/**
 * @brief Establish a MQTT connection.
 * @param[in, out] mqttContext The memory for the MQTTContext_t that will be used for the
 * MQTT connection.
 * @param[in] appMqttCallback The callback function used to receive incoming
 * publishes and incoming acks from MQTT library.
 * @return The MQTT status of the final connection attempt.
 */
static MQTTStatus_t CloudProv_ConnectMQTT(MQTTContext_t * mqttContext, MQTTEventCallback_t appMqttCallback);

/**
 * @brief Function to resend the publishes if a session is re-established with
 * the broker. This function handles the resending of the QoS1 publish packets,
 * which are maintained locally.
 *
 * @param[in] pxMqttContext MQTT context pointer.
 */
static BaseType_t CloudProv_PublishResend(MQTTContext_t * pxMqttContext );

/*************************************************************************************
 * Local Functions
 ************************************************************************************/
static int CloudProv_InitLittleFs(void)
{
    int lfs_err = LFS_ERR_OK;
    fsp_err_t err = FSP_SUCCESS;

    err = RM_LITTLEFS_FLASH_Open(&g_rm_littlefs0_ctrl, &g_rm_littlefs0_cfg);
    if(FSP_SUCCESS != err)
    {
        FAILURE_INDICATION;
        APP_ERR_PRINT("** littleFs Initialization failed **\r\n");
    }

    /* mount the filesystem */
    lfs_err = lfs_mount(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
    if(LFS_ERR_OK != lfs_err)
    {
        /* reformat if we can't mount the filesystem
         * this should only happen on the first boot
         */
        lfs_err = lfs_format(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
        if(LFS_ERR_OK != lfs_err)
        {
            FAILURE_INDICATION;
            APP_ERR_PRINT("** littleFs Flash Format failed **\r\n");
        }
        lfs_err = lfs_mount(&g_rm_littlefs0_lfs, &g_rm_littlefs0_lfs_cfg);
        if(LFS_ERR_OK != lfs_err)
        {
            FAILURE_INDICATION;
            APP_ERR_PRINT("** littleFs Mount failed **\r\n");
        }
    }

    return lfs_err;
}

static void CloudProv_MqttCallback(MQTTContext_t * pxMqttContext,
                                   MQTTPacketInfo_t * pxPacketInfo,
                                   MQTTDeserializedInfo_t * pxDeserializedInfo )
{
    FleetProvisioningStatus_t xStatus;
    FleetProvisioningTopic_t xApi;
    MQTTPublishInfo_t * pxPublishInfo;

    configASSERT( pxMqttContext != NULL );
    configASSERT( pxPacketInfo != NULL );
    configASSERT( pxDeserializedInfo != NULL );

    /* Suppress the unused parameter warning when asserts are disabled in
     * build. */
    ( void ) pxMqttContext;

    /* Handle an incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pxPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
    {
        configASSERT( pxDeserializedInfo->pPublishInfo != NULL );
        pxPublishInfo = pxDeserializedInfo->pPublishInfo;

        xStatus = FleetProvisioning_MatchTopic( pxPublishInfo->pTopicName,
                                                pxPublishInfo->topicNameLength,
                                                &xApi );

        if( xStatus != FleetProvisioningSuccess )
        {
            APP_WARN_PRINT( ( "Unexpected publish message received. Topic: %.*s.\r\n"),
                            ( int ) pxPublishInfo->topicNameLength,
                            ( const char * ) pxPublishInfo->pTopicName );
        }
        else
        {
            /* Copy topic + packet info to global struct. This can be used after the callback */
            memcpy((void *)&CloudProvPublishInfo,
                   (void *)pxDeserializedInfo->pPublishInfo,
                   sizeof(MQTTPublishInfo_t));
            APP_INFO_PRINT( ( "Response from Fleet Provisioning Topic: %.*s.\r\n"),
                            ( int ) pxPublishInfo->topicNameLength,
                            ( const char * ) pxPublishInfo->pTopicName );
            switch (xApi)
            {
                case FleetProvCborCreateCertFromCsrAccepted:
                case FleetProvCborRegisterThingAccepted:
                case FleetProvCborCreateCertFromCsrRejected:
                case FleetProvCborRegisterThingRejected:
                    CloudProvFleetTopic = xApi;
                    break;
                default:
                    APP_ERR_PRINT( ( "Received message on currently unsupported Fleet Provisioning topic.\r\n"));
            }
        }
    }
    else
    {
        /* Handle other packets. */
        switch( pxPacketInfo->type )
        {
            case MQTT_PACKET_TYPE_SUBACK:
                LogInfo( ( "MQTT_PACKET_TYPE_SUBACK.\n\n" ) );
                break;

            case MQTT_PACKET_TYPE_UNSUBACK:
                LogInfo( ( "MQTT_PACKET_TYPE_UNSUBACK.\n\n" ) );
                break;

            case MQTT_PACKET_TYPE_PINGRESP:

                /* Nothing to be done from application as library handles
                 * PINGRESP with the use of MQTT_ProcessLoop API function. */
                LogWarn( ( "PINGRESP should not be handled by the application "
                           "callback when using MQTT_ProcessLoop.\n" ) );
                break;

            case MQTT_PACKET_TYPE_PUBACK:
                LogInfo( ( "PUBACK received for packet id %u.\n\n",
                        pxDeserializedInfo->packetIdentifier ) );
                break;

                /* Any other packet type is invalid. */
            default:
                LogError( ( "Unknown packet type received:(%02x).\n\n",
                        pxPacketInfo->type ) );
        }
    }
}

static uint32_t CloudProv_GetTimeMs(void )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds with FreeRTOS's port macro */
    ulTimeMs = ( uint32_t ) xTickCount * portTICK_PERIOD_MS;

    return ulTimeMs;
}

static TlsTransportStatus_t CloudProv_ConnectTLS(NetworkContext_t * networkContext)
{
    TlsTransportStatus_t connectionStatus = TLS_TRANSPORT_SUCCESS;
    BackoffAlgorithmStatus_t backoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t reconnectParams = {0 };
    NetworkCredentials_t networkCredentials = { 0 };
    uint16_t usNextRetryBackOff = 0U;

#if defined( CLOUD_PROV_CLIENT_USERNAME )

    /* When CLOUD_PROV_CLIENT_USERNAME is defined, use the "mqtt" alpn to connect
     * to AWS IoT Core with Custom Authentication on port 443.
     *
     * Custom Authentication uses the contents of the username and password
     * fields of the MQTT CONNECT packet to authenticate the client.
     *
     * For more information, refer to the documentation at:
     * https://docs.aws.amazon.com/iot/latest/developerguide/custom-authentication.html */
    static const char * ppcAlpnProtocols[] = { "mqtt", NULL };
    #if CLOUD_PROV_MQTT_BROKER_PORT != 443U
    #error "Connections to AWS IoT Core with custom authentication must connect to TCP port 443 with the \"mqtt\" alpn."
    #endif /* CLOUD_PROV_MQTT_BROKER_PORT != 443U */
#else /* if !defined( CLOUD_PROV_CLIENT_USERNAME ) */
    /* Otherwise, use the "x-amzn-mqtt-ca" alpn to connect to AWS IoT Core using
     * x509 Certificate Authentication. */
    static const char * ppcAlpnProtocols[] = { "x-amzn-mqtt-ca", NULL };
#endif /* !defined( CLOUD_PROV_CLIENT_USERNAME ) */

#if (CLOUD_PROV_MQTT_BROKER_PORT == 443U)
    /* An ALPN identifier is only required when connecting to AWS IoT core on port 443.
     * https://docs.aws.amazon.com/iot/latest/developerguide/protocols.html */
    networkCredentials.pAlpnProtos = ppcAlpnProtocols;
#elif (CLOUD_PROV_MQTT_BROKER_PORT == 8883U)
    networkCredentials.pAlpnProtos = NULL;
#else
    #error "MQTT connections to AWS IoT Core are only allowed on ports 443 and 8883."
#endif /* (CLOUD_PROV_MQTT_BROKER_PORT == 443U) */

    /* Set the credentials for establishing a TLS connection. */
    networkCredentials.pRootCa = ( const unsigned char * ) CLOUD_PROV_DEV_ROOT_CA_PEM;
    networkCredentials.rootCaSize = sizeof(CLOUD_PROV_DEV_ROOT_CA_PEM);
    networkCredentials.pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    networkCredentials.pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    networkCredentials.disableSni = pdFALSE;

    /* Initialize backoff algorithm to enable reconnect attempts */
    BackoffAlgorithm_InitializeParams(&reconnectParams,
                                      CLOUD_PROV_TLS_RETRY_BACKOFF_BASE_MS,
                                      CLOUD_PROV_TLS_MAX_BACKOFF_DELAY_MS,
                                      CLOUD_PROV_TLS_RETRY_MAX_ATTEMPTS );

    /* Attempt to connect to MQTT broker. If connection fails, retry after a timeout managed by backoff algorithm */
    do
    {
        /* Establish a TLS connection with the MQTT broker */
        LogInfo( ( "Create a TLS connection to %s:%d.",
                CloudProvMqttEndpoint,
                CLOUD_PROV_MQTT_BROKER_PORT ) );
        connectionStatus = TLS_FreeRTOS_Connect(networkContext,
                                                CloudProvMqttEndpoint,
                                                CLOUD_PROV_MQTT_BROKER_PORT,
                                                &networkCredentials,
                                                CLOUD_PROV_MQTT_SEND_RECV_TIMEOUT_MS,
                                                CLOUD_PROV_MQTT_SEND_RECV_TIMEOUT_MS );

        if(connectionStatus != TLS_TRANSPORT_SUCCESS )
        {
            /* Generate a random number and calculate backoff value (in milliseconds) for
             * the next connection retry.
             * Note: It is recommended to seed the random number generator with a device-specific
             * entropy source so that possibility of multiple devices retrying failed network operations
             * at similar intervals can be avoided. */
            backoffAlgStatus = BackoffAlgorithm_GetNextBackoff(&reconnectParams,
                                                               rand(),
                                                               &usNextRetryBackOff );

            if(backoffAlgStatus == BackoffAlgorithmRetriesExhausted )
            {
                APP_ERR_PRINT( ( "Connection to the broker failed, all attempts exhausted.\r\n" ) );
            }
            else if(backoffAlgStatus == BackoffAlgorithmSuccess )
            {
                APP_WARN_PRINT( ( "Connection to the broker failed. "
                           "Retrying connection with backoff and jitter.\r\n" ) );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
        }
    } while((connectionStatus != TLS_TRANSPORT_SUCCESS ) && (backoffAlgStatus == BackoffAlgorithmSuccess ) );

    return connectionStatus;
}

static MQTTStatus_t CloudProv_ConnectMQTT(MQTTContext_t * mqttContext, MQTTEventCallback_t appMqttCallback)
{
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    MQTTConnectInfo_t connectInfo;
    TransportInterface_t transportInterface;
    TlsTransportStatus_t tlsStatus;
    bsp_unique_id_t const *deviceUniqueId = R_BSP_UniqueIdGet();
    bool sessionPresent = false;
    CK_RV xResult = CKR_OK;

    tlsStatus = CloudProv_ConnectTLS(&CloudProvNetworkContext);

    if(tlsStatus == TLS_TRANSPORT_SUCCESS )
    {
        /* Populate Transport Interface with initialized TCP network context + TLS
         * send and receive function pointers. */
        transportInterface.pNetworkContext = &CloudProvNetworkContext;
        transportInterface.send = TLS_FreeRTOS_send;
        transportInterface.recv = TLS_FreeRTOS_recv;
        transportInterface.writev = NULL;

        /* Initialize MQTT context with TLS transport interface and CloudProv MQTT event callback.
         * This allows a callback specialized for handling Fleet Provisioning topic events */
        mqttStatus = MQTT_Init(mqttContext,
                               &transportInterface,
                               CloudProv_GetTimeMs,
                               appMqttCallback,
                               &CloudProvMqttBuffer);
        if(mqttStatus != MQTTSuccess)
        {
            APP_ERR_PRINT( "MQTT_Init failed with status %s.", MQTT_Status_strerror(mqttStatus ) );
        }
    }

    if(mqttStatus == MQTTSuccess)
    {
        mqttStatus = MQTT_InitStatefulQoS(mqttContext,
                                          CloudProvOutgoingPublishRecords,
                                          CLOUD_PROV_OUTGOING_PUBLISH_RECORD_LEN,
                                          CloudProvIncomingPublishRecords,
                                          CLOUD_PROV_INCOMING_PUBLISH_RECORD_LEN );
        if(mqttStatus != MQTTSuccess)
        {
            APP_ERR_PRINT("MQTT_InitStatefulQos failed with status %s.", MQTT_Status_strerror(mqttStatus));
        }
    }

    if(mqttStatus == MQTTSuccess)
    {
        /* Prepare send CONNECT packet. Init connectInfo struct */
        ( void ) memset(( void * ) &connectInfo, 0x00, sizeof( connectInfo ) );

        /* Start with a clean session i.e. direct the MQTT broker to discard any
         * previous session data. Also, establishing a connection with clean session
         * will ensure that the broker does not store any data when this client
         * gets disconnected. */
        connectInfo.cleanSession = true;

        /* The client identifier is used to uniquely identify this MQTT client to
         * the MQTT broker. In a production device the identifier can be something
         * unique, such as a device serial number. */

        connectInfo.pClientIdentifier = (const char *)deviceUniqueId;
        connectInfo.clientIdentifierLength = ( uint16_t ) CLOUD_PROV_DEVICE_UUID_SIZE_BYTES;
        connectInfo.keepAliveSeconds = CLOUD_PROV_MQTT_KEEP_ALIVE_TIMEOUT_SEC;
#if defined( CLOUD_PROV_CLIENT_USERNAME )
        /* Append metrics string when connecting to AWS IoT Core with custom auth */
        connectInfo.pUserName = CLOUD_PROV_CLIENT_USERNAME AWS_IOT_METRICS_STRING;
        connectInfo.userNameLength = ( uint16_t ) strlen( CLOUD_PROV_CLIENT_USERNAME AWS_IOT_METRICS_STRING );

        /* Use the provided password as-is */
        connectInfo.pPassword = CLOUD_PROV_CLIENT_PASSWORD;
        connectInfo.passwordLength = ( uint16_t ) strlen( CLOUD_PROV_CLIENT_PASSWORD );
#else
        /* If no username is needed, only send the metrics string */
        connectInfo.pUserName = NULL;
        connectInfo.userNameLength =  0u; // ( uint16_t ) strlen( AWS_IOT_METRICS_STRING );
        /* Password for authentication is not used. */
        connectInfo.pPassword = NULL;
        connectInfo.passwordLength = 0U;
#endif /* defined( CLOUD_PROV_CLIENT_USERNAME ) */
        /* Send MQTT CONNECT packet to broker. */
        mqttStatus = MQTT_Connect(mqttContext,
                                  &connectInfo,
                                  NULL,
                                  CLOUD_PROV_MQTT_CONNACK_RECV_TIMEOUT_MS,
                                  &sessionPresent );
        if(mqttStatus == MQTTRecvFailed)
        {
            APP_ERR_PRINT(("TLS connection was done correctly but closed shortly after by AWS IoT Core. "
                           "It is very likely the certificate chain is invalid.\r\n"));
        }
        else if(mqttStatus != MQTTSuccess )
        {
            APP_ERR_PRINT(( "MQTT_Connect() returns status code %s.\r\n"), MQTT_Status_strerror(mqttStatus ));
        }
        else
        {
            /* Connection successfull */
        }
    }

    if(mqttStatus == MQTTSuccess )
    {
        LogInfo( ( "MQTT connection successfully established with broker.\n\n" ) );
    }

    return mqttStatus;
}


static MQTTStatus_t CloudProv_ManageFleetProvTopics(MQTTContext_t *mqttContext, CloudProvTopicAction_t action)
{
    MQTTStatus_t mqttStatus;
    MQTTSubscribeInfo_t subscriptionList[ CLOUD_PROV_FLEET_PROV_TOPIC_COUNT ] = {0u};
    uint16_t packetId;

    /* Populate subscription list with hardcoded topic info */
    subscriptionList[0u].pTopicFilter = FP_CBOR_CREATE_CERT_ACCEPTED_TOPIC;
    subscriptionList[0u].topicFilterLength = FP_CBOR_CREATE_CERT_ACCEPTED_LENGTH;
    subscriptionList[1u].pTopicFilter = FP_CBOR_CREATE_CERT_REJECTED_TOPIC;
    subscriptionList[1u].topicFilterLength = FP_CBOR_CREATE_CERT_REJECTED_LENGTH;
    subscriptionList[2u].pTopicFilter = FP_CBOR_REGISTER_ACCEPTED_TOPIC(CLOUD_PROV_TEMPLATE_NAME );
    subscriptionList[2u].topicFilterLength = FP_CBOR_REGISTER_ACCEPTED_LENGTH(CLOUD_PROV_TEMPLATE_NAME_LENGTH);
    subscriptionList[3u].pTopicFilter = FP_CBOR_REGISTER_REJECTED_TOPIC(CLOUD_PROV_TEMPLATE_NAME );
    subscriptionList[3u].topicFilterLength = FP_CBOR_REGISTER_REJECTED_LENGTH(CLOUD_PROV_TEMPLATE_NAME_LENGTH );

    /* Use QOS1 for all the topics */
    for(uint8_t topic=0u; topic < CLOUD_PROV_FLEET_PROV_TOPIC_COUNT; topic++)
    {
        subscriptionList[topic].qos = MQTTQoS1;
    }

    /* Generate packet identifier for the SUBSCRIBE packet. */
    packetId = MQTT_GetPacketId( mqttContext );

    /* Send SUBSCRIBE packet. */
    if(action == CloudProv_Subscribe)
    {
        mqttStatus = MQTT_Subscribe(mqttContext,
                                    subscriptionList,
                                    CLOUD_PROV_FLEET_PROV_TOPIC_COUNT,
                                    packetId );
    }
    else if(action == CloudProv_Unsub)
    {
        mqttStatus = MQTT_Unsubscribe(mqttContext,
                                      subscriptionList,
                                      CLOUD_PROV_FLEET_PROV_TOPIC_COUNT,
                                      packetId );
    }
    else
    {
        LogError( ("Should not get here, please call an ambulance, the programmer is insane"));
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

static bool CloudProv_RequestCertificate(MQTTContext_t *mqttContext,
                                  uint8_t *ownershipToken,
                                  size_t *ownershipTokenLength)
{
    uint8_t certBuffer[ CLOUD_PROV_CERT_BUFFER_SIZE ] = {0 };
    uint8_t payloadBuffer[CLOUD_PROV_MQTT_BUFFER_SIZE];
    size_t certLength = 0u;
    uint8_t certIdBuffer[CLOUD_PROV_CERT_ID_BUFFER_SIZE];
    size_t certIdLength = CLOUD_PROV_CERT_ID_BUFFER_SIZE;
    size_t payloadLength = 0u;
    bool status = false;
    MQTTStatus_t mqttStatus = MQTTBadParameter;

    status = CloudProv_GenerateCsr(CloudProvP11Session,
                                   certBuffer,
                                   CLOUD_PROV_CERT_BUFFER_SIZE,
                                   &certLength);
    if(status == true)
    {
        status = CloudProv_SerializeCsr(certBuffer,
                                        certLength,
                                        CLOUD_PROV_MQTT_BUFFER_SIZE,
                                        payloadBuffer,
                                        &payloadLength);
    }

    if(status == true)
    {

        uint16_t packetId = MQTT_GetPacketId(mqttContext);
        MQTTPublishInfo_t pubInfo = {
            .pTopicName = FP_CBOR_CREATE_CERT_PUBLISH_TOPIC,
            .topicNameLength = FP_CBOR_CREATE_CERT_PUBLISH_LENGTH,
            .pPayload = payloadBuffer,
            .payloadLength = payloadLength,
            .qos = MQTTQoS1
            };

        mqttStatus = MQTT_Publish(mqttContext, &pubInfo, packetId);
        if(mqttStatus != MQTTSuccess)
        {
            LogError( ( "Failed to publish to fleet provisioning topic: %.*s.\r\n",
                    FP_CBOR_CREATE_CERT_PUBLISH_LENGTH,
                    FP_CBOR_CREATE_CERT_PUBLISH_TOPIC ));
            status = false;
        }
    }

    if(mqttStatus == MQTTSuccess)
    {
        /* increase wait time here because Server takes a long time to write CSR and send response */
        uint16_t timeMs = 0u;
        while((timeMs <= 5000u) && (CloudProvFleetTopic == FleetProvisioningInvalidTopic))
        {
            mqttStatus = MQTT_ProcessLoop( mqttContext);
            timeMs += 1000u;
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
    }

    if((mqttStatus == MQTTSuccess) && (CloudProvFleetTopic == FleetProvCborCreateCertFromCsrAccepted))
    {
        /* From the response, extract the certificate, certificate ID, and
         * certificate ownership token. */
        /* set certLength to buffer size, since this is an input/output parameter that takes max buff
         * size as input and gives string length written into buffer as output */
        certLength = CLOUD_PROV_CERT_BUFFER_SIZE;
        status = CloudProv_DeserializeCsrResponse((const uint8_t *)CloudProvPublishInfo.pPayload,
                                                   CloudProvPublishInfo.payloadLength,
                                                   (char *)certBuffer,
                                                   &certLength,
                                                   (char *)certIdBuffer,
                                                   &certIdLength,
                                                   (char *)ownershipToken,
                                                   ownershipTokenLength);
        if(status == true)
        {
            status = CloudProv_LoadCertificate(CloudProvP11Session,
                                               (const char *)certBuffer,
                                               certLength);
        }
    }
    else
    {
        status = false;
    }

    /* Reset cloud provision fleet topic for next publish */
    CloudProvFleetTopic = FleetProvisioningInvalidTopic;
    return status;
}

static MQTTStatus_t CloudProv_RegisterDevice(MQTTContext_t *mqttContext,
                                  uint8_t *ownershipToken,
                                  size_t ownershipTokenLength)
{
    uint8_t payloadBuffer[CLOUD_PROV_MQTT_BUFFER_SIZE];
    size_t payloadLength;
    bsp_unique_id_t const *deviceUniqueId = R_BSP_UniqueIdGet();
    CborError cborStatus;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    size_t thingNameLength = CLOUD_PROV_THING_NAME_BUFFER_SIZE;
    char deviceId[36u];
    sprintf(deviceId, (void *) "%08x-%08x-%08x-%08x",
            (uint32_t) deviceUniqueId->unique_id_words[0], (uint32_t) deviceUniqueId->unique_id_words[1],
            (uint32_t) deviceUniqueId->unique_id_words[2], (uint32_t) deviceUniqueId->unique_id_words[3]);
    APP_INFO_PRINT( ( "Device Unique ID : %s\r\n"),deviceId );

    cborStatus = CloudProv_SerializeRegisterThingRequest( (const char *)ownershipToken,
                                             ownershipTokenLength,
                                                          deviceId,
                                                          strlen(deviceId),
                                            CLOUD_PROV_MQTT_BUFFER_SIZE,
                                            payloadBuffer,
                                            &payloadLength );

    if(cborStatus == CborNoError)
    {
        uint16_t packetId = MQTT_GetPacketId(mqttContext);
        MQTTPublishInfo_t pubInfo = {
                .pTopicName = FP_CBOR_REGISTER_PUBLISH_TOPIC(CLOUD_PROV_TEMPLATE_NAME ),
                .topicNameLength = FP_CBOR_REGISTER_PUBLISH_LENGTH(CLOUD_PROV_TEMPLATE_NAME_LENGTH ),
                .pPayload = payloadBuffer,
                .payloadLength = payloadLength,
                .qos = MQTTQoS1
        };
        mqttStatus = MQTT_Publish(mqttContext, &pubInfo, packetId);
        if(mqttStatus != MQTTSuccess)
        {
            LogError( ( "Failed to publish to fleet provisioning topic: %.*s.\r\n",
                    FP_CBOR_REGISTER_PUBLISH_LENGTH(CLOUD_PROV_TEMPLATE_NAME_LENGTH ),
                    FP_CBOR_REGISTER_PUBLISH_TOPIC(CLOUD_PROV_TEMPLATE_NAME ) ));
        }
    }

    if(mqttStatus == MQTTSuccess)
    {
        /* increase wait time here because Server takes a long time to write CSR and send response */
        uint16_t timeMs = 0u;
        while((timeMs <= 5000u) && (CloudProvFleetTopic == FleetProvisioningInvalidTopic))
        {
            mqttStatus = MQTT_ProcessLoop( mqttContext);
            timeMs += 1000u;
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
    }

    if((mqttStatus == MQTTSuccess) && (CloudProvFleetTopic == FleetProvCborRegisterThingAccepted))
    {
        cborStatus = CloudProv_DeserializeThingName((const uint8_t *)CloudProvPublishInfo.pPayload,
                                       CloudProvPublishInfo.payloadLength,
                                       CloudProvThingName,
                                       &thingNameLength);
        if(cborStatus == CborNoError)
        {
            APP_INFO_PRINT( ( "Received AWS IoT Thing name: %.*s\r\n"), ( int ) thingNameLength, CloudProvThingName );

        }
        else
        {
            /* Must provide feedback that something failed. Since mqtt didn't fail, the most generic error is sent
             * to signify that somethign went wrong (cbor in this case) */
            mqttStatus = MQTTBadParameter;
        }
    }
    else
    {
        /* Register thing was not accepted, so return a failed status */
        mqttStatus = MQTTBadParameter;
    }
    return mqttStatus;
}


/*************************************************************************************
 * global functions
 ************************************************************************************/

MQTTStatus_t CloudProv_ProvisionDevice(MQTTContext_t *mqttContext, MQTTEventCallback_t mqttCallback)
{
    fsp_err_t fspError;
    CK_RV xPkcs11Ret = CKR_CANCEL;
    CK_OBJECT_HANDLE certHandle = 0u;
    CK_OBJECT_HANDLE pkHandle = 0u;
    MQTTStatus_t mqttStatus = MQTTRecvFailed;
    bool connected = false;
    bool status = false;
    uint8_t ownershipToken[ CLOUD_PROV_OWNERSHIP_TOKEN_BUFFER_SIZE ];
    size_t ownershipTokenLength = CLOUD_PROV_OWNERSHIP_TOKEN_BUFFER_SIZE;


    xPkcs11Ret = xDestroyDefaultCryptoObjects(CloudProvP11Session );
    if(xPkcs11Ret != CKR_OK)
    {
        APP_ERR_PRINT( ( "Failed to Destroy corePKCS11 Crypto Objects.\r\n" ) );
    }

    if( xPkcs11Ret == CKR_OK )
    {
        /* Provision Claim Private Key */
        xPkcs11Ret = xProvisionPrivateKey(CloudProvP11Session,
                                          (unsigned char *) CloudProvClaimPrivateKey,
                                          strlen(CloudProvClaimPrivateKey) + 1,
                                          ( uint8_t * ) pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
                                          &pkHandle );
        if(xPkcs11Ret != CKR_OK)
        {
            APP_WARN_PRINT( ( "Failed to import claim private key to corePKCS11.\r\n" ) );
        }
    }

    if( xPkcs11Ret == CKR_OK )
    {
        /* Provision Claim Certificate */
        xPkcs11Ret = xProvisionCertificate(CloudProvP11Session,
                                           (unsigned char *) CloudProvClaimCert,
                                            1 + strlen(CloudProvClaimCert),
                                           ( uint8_t * ) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
                                           &certHandle );
        if(xPkcs11Ret != CKR_OK)
        {
            APP_WARN_PRINT( ( "Failed to import claim certificate to corePKCS11.\r\n" ) );
        }
    }


    if( xPkcs11Ret == CKR_OK )
    {
        /* Try to connect to MQTT broker with claim credentials */
        APP_INFO_PRINT( ( "Trying to connect to MQTT broker with claim credentials to provision Cloud Kit...\r\n" ) );
        mqttStatus = CloudProv_ConnectMQTT(mqttContext, CloudProv_MqttCallback);
    }

    if(mqttStatus == MQTTSuccess)
    {
        connected = true;
        /* Subscribe to Fleet Provisioning MQTT topics */
        mqttStatus = CloudProv_ManageFleetProvTopics(mqttContext, CloudProv_Subscribe);
    }

    if(mqttStatus == MQTTSuccess)
    {
        /* Request a certificate from AWS IoT and store it  */
        status = CloudProv_RequestCertificate(mqttContext, ownershipToken, &ownershipTokenLength);
    }

    if(status == true)
    {
        mqttStatus = CloudProv_RegisterDevice(mqttContext,ownershipToken,ownershipTokenLength);
    }

    if((status == true) && (mqttStatus == MQTTSuccess))
    {
        /* Unsubscribe to Fleet Provisioning MQTT topics */
        mqttStatus = CloudProv_ManageFleetProvTopics(mqttContext, CloudProv_Unsub);
    }

    /* Send MQTT DISCONNECT. */
    if(connected == true)
    {
        MQTT_Disconnect( mqttContext );
    }
    /* Close TLS connection.  */
    TLS_FreeRTOS_Disconnect( &CloudProvNetworkContext );

    if((status == true) && (mqttStatus == MQTTSuccess))
    {
        /* Reconnect with new generated device credentials */
        mqttStatus = CloudProv_ConnectMQTT(mqttContext, mqttCallback);

        /* Try to read incoming packets for come seconds, in case the TLS connection is cut by the client
         * in case of bad chain of certificate */
        uint16_t timeMs = 0u;
        while((timeMs <= 2000u))
        {
            mqttStatus = MQTT_ProcessLoop( mqttContext);
            timeMs += 1000u;
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
        if(mqttStatus == MQTTRecvFailed)
        {
            APP_WARN_PRINT( ( "There is a strong possibility that the certificate chain is invalid. "\
                                    "Make sure the claim certificate + claim private key + root CA are correctly "\
                                  "associated to a provision template on AWS IoT.\r\n"),
                            MQTT_Status_strerror(mqttStatus) );
        }
    }

    if((status != true) || (mqttStatus != MQTTSuccess))
    {
        APP_ERR_PRINT(("Could not Provision device on AWS IoT Server. \r\n"));
        mqttStatus = MQTTServerRefused;
    }

    return mqttStatus;
}

MQTTStatus_t CloudProv_Init(MQTTContext_t * mqttContext, MQTTEventCallback_t appMqttCallback)
{
    uint8_t lfsStatus = LFS_ERR_CORRUPT;
    MQTTStatus_t mqttStatus = MQTTBadParameter;
    fsp_err_t fspError = FSP_ERR_ASSERTION;
    CK_RV   pkcs11status = CKR_GENERAL_ERROR;
    uint32_t ipAddress = 0u;

    /* Set MQTT context in known state */
    memset(mqttContext, 0x00, sizeof(MQTTContext_t ));

    /* Initialize littleFS to store crypto secrets with corePKCS11 */
    lfsStatus = CloudProv_InitLittleFs();

    if(lfsStatus == LFS_ERR_OK)
    {
        /* Initialize the PKCS #11 module */
        mbedtls_platform_setup(NULL);
        pkcs11status = xInitializePkcs11Session( &CloudProvP11Session );
    }

    if(pkcs11status == CKR_OK)
    {
        /* Initialize FreeRTOS's IP network stack */
        CloudProv_InitIPStack();
        /* Get the IP address for the MQTT END POINT used for the application */
        ipAddress = FreeRTOS_gethostbyname((char*)CloudProvMqttEndpoint);
    }

    if(0u == ipAddress)
    {
        FAILURE_INDICATION;
        APP_WARN_PRINT("FreeRTOS_gethostbyname() Failed to get IP address from End point address for %s\r\n\r\n",
                      CloudProvMqttEndpoint);
        while(1)
        {
            APP_WARN_PRINT("MQTT Broker endpoint is not reachable"
                           "\r\nPlease reset Cloud Kit [ORANGE]while spamming BACKSPACE KEY[YELLOW] "
                           "to open MENU and try new MQTT Broker endpoint\r\n\r\n");
            vTaskDelay(10000);
        }
    }
    else
    {
        /* Convert the IP address to a string to print on to the console. */
        FreeRTOS_inet_ntoa(ipAddress, ( char * ) cBuffer);
        APP_PRINT("\r\nDNS Lookup for \"%s\" is      : %s  \r\n", CloudProvMqttEndpoint, cBuffer);

        if(CLoudProvForceProvisioning == false)
        {
            /* Assume the device is provisioned already and try to connect to MQTT with MQTT Broker
             * endpoint + Device credentials stored in corePKCS11. If connection fails, because credentials
             * are bad for example, the fault status is returned and caller is responsible to call
             * CloudProv_ProvisionDevice to try again nwith new credentials */
            APP_INFO_PRINT( ( "Assuming Cloud Kit is already provisioned, trying to connect to "
                              "MQTT broker with device credentials... \r\n") );
            mqttStatus = CloudProv_ConnectMQTT(mqttContext, appMqttCallback);
        }
        else
        {
            /* Force Provisioning of device, thus do not attempt to connect MQTT with credentials
             * stored in corePKCS11. */
            mqttStatus = MQTTBadParameter;
        }

        if(mqttStatus != MQTTSuccess)
        {
            /* Connection to MQTT was unsuccessful. This might be caused by the certificate chain being invalid,
             * Thus, try to provision device via fleet provisioning */
            mqttStatus = CloudProv_ProvisionDevice(mqttContext, appMqttCallback);
            if(mqttStatus != MQTTSuccess)
            {
                APP_WARN_PRINT("CloudApp could not authenticate with given claim credentials."
                               "\r\nPlease reset Cloud Kit [ORANGE]while spamming BACKSPACE KEY[YELLOW] "
                               "to open MENU and try new credentials\r\n\r\n");
            }
        }
    }

    return mqttStatus;
}

uint8_t CloudProv_ImportMqttEndpoint(uint8_t *endpointBuffer, size_t endpointLength, bool forceProvisioning)
{
    uint8_t status = 0u;

    if(endpointLength < CLOUD_PROV_MQTT_ENDPOINT_BUFFER_SIZE)
    {
        memset(CloudProvMqttEndpoint, 0, strlen (CloudProvMqttEndpoint));
        memcpy((void*)CloudProvMqttEndpoint, (void *)endpointBuffer, endpointLength);
        CLoudProvForceProvisioning = forceProvisioning;
    }
    else
    {
        APP_ERR_PRINT("\r\nCannot import MQTT Broker Endpoint into CloudProv module, buffer insufficient");
        status = 1u;
    }
    return status;
}

void CloudProv_ForceProvisioning(void)
{
    CLoudProvForceProvisioning = true;
}

uint8_t CloudProv_ImportClaimCertificate(uint8_t *endpointBuffer, size_t endpointLength, bool forceProvisioning)
{
    uint8_t status = 0u;

    if(endpointLength < CLOUD_PROV_CLAIM_CERT_BUFFER_SIZE)
    {
        memset(CloudProvClaimCert, 0, strlen (CloudProvClaimCert));
        memcpy((void*)CloudProvClaimCert, (void *)endpointBuffer, endpointLength);
        CLoudProvForceProvisioning = forceProvisioning;
    }
    else
    {
        APP_ERR_PRINT("\r\nCannot import Claim Certificate into CloudProv module, buffer insufficient");
        status = 1u;
    }
    return status;
}

uint8_t CloudProv_ImportClaimPrivateKey(uint8_t *endpointBuffer, size_t endpointLength, bool forceProvisioning)
{
    uint8_t status = 0u;

    if(endpointLength <  CLOUD_PROV_CLAIM_PRIVATE_KEY_BUFFER_SIZE)
    {
        memset(CloudProvClaimPrivateKey, 0, strlen (CloudProvClaimPrivateKey));
        memcpy((void*)CloudProvClaimPrivateKey, (void *)endpointBuffer, endpointLength);
        CLoudProvForceProvisioning = forceProvisioning;
    }
    else
    {
        APP_ERR_PRINT("\r\nCannot import Claim Private Key into CloudProv module, buffer insufficient");
        status = 1u;
    }
    return status;
}
