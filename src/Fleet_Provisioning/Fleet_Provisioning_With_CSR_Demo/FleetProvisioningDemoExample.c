/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * Demo for showing use of the Fleet Provisioning library to use the Fleet
 * Provisioning feature of AWS IoT Core for provisioning devices with
 * credentials. This demo shows how a device can be provisioned with AWS IoT
 * Core using the Certificate Signing Request workflow of the Fleet
 * Provisioning feature.
 *
 * The Fleet Provisioning library provides macros and helper functions for
 * assembling MQTT topics strings, and for determining whether an incoming MQTT
 * message is related to the Fleet Provisioning API of AWS IoT Core. The Fleet
 * Provisioning library does not depend on any particular MQTT library,
 * therefore the functionality for MQTT operations is placed in another file
 * (mqtt_operations.c). This demo uses the coreMQTT library. If needed,
 * mqtt_operations.c can be modified to replace coreMQTT with another MQTT
 * library. This demo requires using the AWS IoT Core broker as Fleet
 * Provisioning is an AWS IoT Core feature.
 *
 * This demo provisions a device certificate using the provisioning by claim
 * workflow with a Certificate Signing Request (CSR). The demo connects to AWS
 * IoT Core using provided claim credentials (whose certificate needs to be
 * registered with IoT Core before running this demo), subscribes to the
 * CreateCertificateFromCsr topics, and obtains a certificate. It then
 * subscribes to the RegisterThing topics and activates the certificate and
 * obtains a Thing using the provisioning template. Finally, it reconnects to
 * AWS IoT Core using the new credentials.
 */

/* Debug includes */
#include "common_utils.h"

/* AWS IoT Fleet Provisioning Library. */
#include "fleet_provisioning.h"

/* Demo includes. */
#include "aws_dev_mode_key_provisioning.h"
#include "mqtt_pkcs11_demo_helpers.h"
#include "pkcs11_operations.h"
#include "tinycbor_serializer.h"
#include "cbor.h"

/* Demo Config */
#include "demo_config.h"

#include "usr_network.h"
#include "usr_hal.h"
#include "usr_config.h"

/**
 * These configurations are required. Throw compilation error if it is not
 * defined.
 */
#ifndef democonfigPROVISIONING_TEMPLATE_NAME
    #error "Please define democonfigPROVISIONING_TEMPLATE_NAME to the template name registered with AWS IoT Core in demo_config.h."
#endif
#ifndef democonfigROOT_CA_PEM
    #error "Please define Root CA certificate of the MQTT broker(democonfigROOT_CA_PEM) in demo_config.h."
#endif

/**
 * @brief The length of #democonfigPROVISIONING_TEMPLATE_NAME.
 */
#define fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH    ( ( uint16_t ) ( sizeof( democonfigPROVISIONING_TEMPLATE_NAME ) - 1 ) )

/**
 * @brief The length of #democonfigFP_DEMO_ID.
 */
#define fpdemoFP_DEMO_ID_LENGTH                    ( ( uint16_t ) ( sizeof( democonfigFP_DEMO_ID ) - 1 ) )

/**
 * @brief Size of AWS IoT Thing name buffer.
 *
 * See https://docs.aws.amazon.com/iot/latest/apireference/API_CreateThing.html#iot-CreateThing-request-thingName
 */
#define fpdemoMAX_THING_NAME_LENGTH                128

/**
 * @brief The maximum number of times to run the loop in this demo.
 *
 * @note The demo loop is attempted to re-run only if it fails in an iteration.
 * Once the demo loop succeeds in an iteration, the demo exits successfully.
 */
#ifndef fpdemoMAX_DEMO_LOOP_COUNT
    #define fpdemoMAX_DEMO_LOOP_COUNT    ( 3 )
#endif

/**
 * @brief Time in seconds to wait between retries of the demo loop if
 * demo loop fails.
 */
#define fpdemoDELAY_BETWEEN_DEMO_RETRY_ITERATIONS_SECONDS    ( 10 )

/**
 * @brief Size of buffer in which to hold the certificate signing request (CSR).
 */
#define fpdemoCSR_BUFFER_LENGTH                              4096

/**
 * @brief Size of buffer in which to hold the certificate.
 */
#define fpdemoCERT_BUFFER_LENGTH                             4096

/**
 * @brief Size of buffer in which to hold the certificate id.
 *
 * @note Has a maximum length of 64 for more information see the following link
 * https://docs.aws.amazon.com/iot/latest/apireference/API_Certificate.html#iot-Type-Certificate-certificateId
 */
#define fpdemoCERT_ID_BUFFER_LENGTH                          64

/**
 * @brief Size of buffer in which to hold the certificate ownership token.
 */
#define fpdemoOWNERSHIP_TOKEN_BUFFER_LENGTH                  1024

/**
 * @brief Milliseconds per second.
 */
#define fpdemoMILLISECONDS_PER_SECOND                        ( 1000U )

/**
 * @brief Milliseconds per FreeRTOS tick.
 */
#define fpdemoMILLISECONDS_PER_TICK                          ( fpdemoMILLISECONDS_PER_SECOND / configTICK_RATE_HZ )

/**
 * @brief Status values of the Fleet Provisioning response.
 */
typedef enum
{
    ResponseNotReceived,
    ResponseAccepted,
    ResponseRejected
} ResponseStatus_t;


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

/*-----------------------------------------------------------*/

/**
 * @brief Status reported from the MQTT publish callback.
 */
static ResponseStatus_t xResponseStatus;

/**
 * @brief Buffer to hold the provisioned AWS IoT Thing name.
 */
static char pcThingName[ fpdemoMAX_THING_NAME_LENGTH ];

/**
 * @brief Length of the AWS IoT Thing name.
 */
static size_t xThingNameLength;

/**
 * @brief Buffer to hold responses received from the AWS IoT Fleet Provisioning
 * APIs. When the MQTT publish callback receives an expected Fleet Provisioning
 * accepted payload, it copies it into this buffer.
 */
static uint8_t pucPayloadBuffer[ democonfigNETWORK_BUFFER_SIZE * 2 ];

/**
 * @brief Length of the payload stored in #pucPayloadBuffer. This is set by the
 * MQTT publish callback when it copies a received payload into #pucPayloadBuffer.
 */
static size_t xPayloadLength;

/**
 * @brief The MQTT context used for MQTT operation.
 */
static MQTTContext_t xMqttContext;

/**
 * @brief The network context used for mbedTLS operation.
 */
static NetworkContext_t xNetworkContext;

/**
 * @brief The parameters for the network context using mbedTLS operation.
 */
static TlsTransportParams_t xTlsTransportParams;

/**
 * @brief Static buffer used to hold MQTT messages being sent and received.
 */
static uint8_t ucSharedBuffer[ democonfigNETWORK_BUFFER_SIZE ];

char claimCertificatePem[2048];

char claimKeyPem[2048];

/**
 * @brief Static buffer used to hold MQTT messages being sent and received.
 */
static MQTTFixedBuffer_t xBuffer =
{
    ucSharedBuffer,
    democonfigNETWORK_BUFFER_SIZE
};

extern uint32_t g_console_status;

/*-----------------------------------------------------------*/

/**
 * @brief Callback to receive the incoming publish messages from the MQTT
 * broker. Sets xResponseStatus if an expected CreateCertificateFromCsr or
 * RegisterThing response is received, and copies the response into
 * responseBuffer if the response is an accepted one.
 *
 * @param[in] pPublishInfo Pointer to publish info of the incoming publish.
 * @param[in] usPacketIdentifier Packet identifier of the incoming publish.
 */
static void prvProvisioningPublishCallback( MQTTContext_t * pxMqttContext,
                                            MQTTPacketInfo_t * pxPacketInfo,
                                            MQTTDeserializedInfo_t * pxDeserializedInfo );

/**
 * @brief Subscribe to the CreateCertificateFromCsr accepted and rejected topics.
 */
static bool prvSubscribeToCsrResponseTopics( void );

/**
 * @brief Unsubscribe from the CreateCertificateFromCsr accepted and rejected topics.
 */
static bool prvUnsubscribeFromCsrResponseTopics( void );

/**
 * @brief Subscribe to the RegisterThing accepted and rejected topics.
 */
static bool prvSubscribeToRegisterThingResponseTopics( void );

/**
 * @brief Unsubscribe from the RegisterThing accepted and rejected topics.
 */
static bool prvUnsubscribeFromRegisterThingResponseTopics( void );

/**
 * @brief The task used to demonstrate the FP API.
 *
 * This task uses the provided claim key and certificate files to connect to
 * AWS and use PKCS #11 to generate a new device key and certificate with a CSR.
 * The task then creates a new Thing with the Fleet Provisioning API using the
 * newly-created credentials. The task finishes by connecting to the newly-created
 * Thing to verify that it was successfully created and accessible using the key/cert.
 *
 * @param[in] pvParameters Parameters as passed at the time of task creation.
 * Not used in this example.
 */
int prvFleetProvisioningTask();


/*-----------------------------------------------------------*/

static void prvProvisioningPublishCallback( MQTTContext_t * pxMqttContext,
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
            if( xApi == FleetProvCborCreateCertFromCsrAccepted )
            {
            	APP_INFO_PRINT( ( "Received accepted response from Fleet Provisioning CreateCertificateFromCsr API: %.*s.\r\n"),
                            ( int ) pxPublishInfo->topicNameLength,
                            ( const char * ) pxPublishInfo->pTopicName );

                xResponseStatus = ResponseAccepted;

                /* Copy the payload from the MQTT library's buffer to #pucPayloadBuffer. */
                ( void ) memcpy( ( void * ) pucPayloadBuffer,
                                 ( const void * ) pxPublishInfo->pPayload,
                                 ( size_t ) pxPublishInfo->payloadLength );

                xPayloadLength = pxPublishInfo->payloadLength;
            }
            else if( xApi == FleetProvCborCreateCertFromCsrRejected )
            {
                APP_ERR_PRINT( ( "Received rejected response from Fleet Provisioning CreateCertificateFromCsr API: %.*s.\r\n"),
                        ( int ) pxPublishInfo->topicNameLength,
                        ( const char * ) pxPublishInfo->pTopicName);

                xResponseStatus = ResponseRejected;
            }
            else if( xApi == FleetProvCborRegisterThingAccepted )
            {
            	APP_INFO_PRINT( ( "Received accepted response from Fleet Provisioning RegisterThing API.\r\n" ) );

                xResponseStatus = ResponseAccepted;

                /* Copy the payload from the MQTT library's buffer to #pucPayloadBuffer. */
                ( void ) memcpy( ( void * ) pucPayloadBuffer,
                                 ( const void * ) pxPublishInfo->pPayload,
                                 ( size_t ) pxPublishInfo->payloadLength );

                xPayloadLength = pxPublishInfo->payloadLength;
            }
            else if( xApi == FleetProvCborRegisterThingRejected )
            {
                APP_ERR_PRINT( ( "Received rejected response from Fleet Provisioning RegisterThing API.: %.*s.\r\n"),
                        ( int ) pxPublishInfo->topicNameLength,
                        ( const char * ) pxPublishInfo->pTopicName);
                APP_ERR_PRINT( ( "%.*s.\r\n"),
                                        ( int ) pxPublishInfo->payloadLength,
                                        ( const char * ) pxPublishInfo->pPayload);

                xResponseStatus = ResponseRejected;
            }
            else
            {
                APP_ERR_PRINT( ( "Received message on unexpected Fleet Provisioning topic. Topic: %.*s.\r\n"),
                            ( int ) pxPublishInfo->topicNameLength,
                            ( const char * ) pxPublishInfo->pTopicName );
            }
        }
    }
    else
    {
        vHandleOtherIncomingPacket( pxPacketInfo, pxDeserializedInfo->packetIdentifier );
        xResponseStatus = ResponseAccepted;
    }
}
/*-----------------------------------------------------------*/

static bool prvSubscribeToCsrResponseTopics( void )
{
    bool xStatus;

    xStatus = xSubscribeToTopic( &xMqttContext,
                                 FP_CBOR_CREATE_CERT_ACCEPTED_TOPIC,
                                 FP_CBOR_CREATE_CERT_ACCEPTED_LENGTH );

    if( xStatus == false )
    {
        APP_ERR_PRINT( ( "Failed to subscribe to fleet provisioning topic: %.*s.\r\n"),
                    FP_CBOR_CREATE_CERT_ACCEPTED_LENGTH,
                    FP_CBOR_CREATE_CERT_ACCEPTED_TOPIC);
    }

    if( xStatus == true )
    {
        xStatus = xSubscribeToTopic( &xMqttContext,
                                     FP_CBOR_CREATE_CERT_REJECTED_TOPIC,
                                     FP_CBOR_CREATE_CERT_REJECTED_LENGTH );

        if( xStatus == false )
        {
            APP_ERR_PRINT( ( "Failed to subscribe to fleet provisioning topic: %.*s.\r\n"),
                        FP_CBOR_CREATE_CERT_REJECTED_LENGTH,
                        FP_CBOR_CREATE_CERT_REJECTED_TOPIC);
        }
    }

    return xStatus;
}
/*-----------------------------------------------------------*/

static bool prvUnsubscribeFromCsrResponseTopics( void )
{
    bool xStatus;

    xStatus = xUnsubscribeFromTopic( &xMqttContext,
                                     FP_CBOR_CREATE_CERT_ACCEPTED_TOPIC,
                                     FP_CBOR_CREATE_CERT_ACCEPTED_LENGTH );

    if( xStatus == false )
    {
        APP_ERR_PRINT( ( "Failed to unsubscribe from fleet provisioning topic: %.*s.\r\n"),
                    FP_CBOR_CREATE_CERT_ACCEPTED_LENGTH,
                    FP_CBOR_CREATE_CERT_ACCEPTED_TOPIC);
    }

    if( xStatus == true )
    {
        xStatus = xUnsubscribeFromTopic( &xMqttContext,
                                         FP_CBOR_CREATE_CERT_REJECTED_TOPIC,
                                         FP_CBOR_CREATE_CERT_REJECTED_LENGTH );

        if( xStatus == false )
        {
            APP_ERR_PRINT( ( "Failed to unsubscribe from fleet provisioning topic: %.*s.\r\n"),
                        FP_CBOR_CREATE_CERT_REJECTED_LENGTH,
                        FP_CBOR_CREATE_CERT_REJECTED_TOPIC );
        }
    }

    return xStatus;
}
/*-----------------------------------------------------------*/

static bool prvSubscribeToRegisterThingResponseTopics( void )
{
    bool xStatus;

    xStatus = xSubscribeToTopic( &xMqttContext,
                                 FP_CBOR_REGISTER_ACCEPTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ),
                                 FP_CBOR_REGISTER_ACCEPTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ) );

    if( xStatus == false )
    {
        APP_ERR_PRINT( ( "Failed to subscribe to fleet provisioning topic: %.*s.\r\n"),
                    FP_CBOR_REGISTER_ACCEPTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                    FP_CBOR_REGISTER_ACCEPTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME) );
    }

    if( xStatus == true )
    {
        xStatus = xSubscribeToTopic( &xMqttContext,
                                     FP_CBOR_REGISTER_REJECTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ),
                                     FP_CBOR_REGISTER_REJECTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ) );

        if( xStatus == false )
        {
            APP_ERR_PRINT( ( "Failed to subscribe to fleet provisioning topic: %.*s.\r\n"),
                        FP_CBOR_REGISTER_REJECTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                        FP_CBOR_REGISTER_REJECTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME) );
        }
    }

    return xStatus;
}
/*-----------------------------------------------------------*/

static bool prvUnsubscribeFromRegisterThingResponseTopics( void )
{
    bool xStatus;

    xStatus = xUnsubscribeFromTopic( &xMqttContext,
                                     FP_CBOR_REGISTER_ACCEPTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ),
                                     FP_CBOR_REGISTER_ACCEPTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ) );

    if( xStatus == false )
    {
        APP_ERR_PRINT( ( "Failed to unsubscribe from fleet provisioning topic: %.*s.\r\n"),
                    FP_CBOR_REGISTER_ACCEPTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                    FP_CBOR_REGISTER_ACCEPTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ) );
    }

    if( xStatus == true )
    {
        xStatus = xUnsubscribeFromTopic( &xMqttContext,
                                         FP_CBOR_REGISTER_REJECTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ),
                                         FP_CBOR_REGISTER_REJECTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ) );

        if( xStatus == false )
        {
            APP_ERR_PRINT( ( "Failed to unsubscribe from fleet provisioning topic: %.*s.\r\n"),
                        FP_CBOR_REGISTER_REJECTED_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                        FP_CBOR_REGISTER_REJECTED_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ) );
        }
    }

    return xStatus;
}

/*********************************************************************************************************************//**
 * @brief   configures the littleFS Flash.
 *
 * This function sets up the littleFS Flash for Data storage.
 * @param[in]   None
 * @retval      LFS_ERR_OK              If both the connectivity checks are success.
 * @retval      Any other error         If one of the connectivity check fails.
 *********************************************************************************************************************/
static int config_littlFs_flash(void)
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

/*-----------------------------------------------------------*/

/* This example uses a single application task, which shows that how to use
 * the Fleet Provisioning library to generate and validate AWS IoT Fleet
 * Provisioning MQTT topics, and use the coreMQTT library to communicate with
 * the AWS IoT Fleet Provisioning APIs. */
void FleetProvisioningDemo(void *pvParameters)
{
	fsp_pack_version_t      version = {RESET_VALUE};
	fsp_err_t               err = FSP_SUCCESS;
	BaseType_t              bt_status = pdFALSE;
	int                     lfs_err = LFS_ERR_OK;
	int                     ierr = FSP_SUCCESS;
	uint32_t                ip_status = RESET_VALUE;
	const char              * pcTopicFilter = NULL;
	uint16_t                topicFilterLength = RESET_VALUE;
	static uint32_t   sequenceNumber = RESET_VALUE;
    bool xStatus = false;
    /* Buffer for holding the CSR. */
    char pcCsr[ fpdemoCSR_BUFFER_LENGTH ] = { 0 };
    size_t xCsrLength = 0;
    /* Buffer for holding received certificate until it is saved. */
    char pcCertificate[ fpdemoCERT_BUFFER_LENGTH ];
    size_t xCertificateLength;
    /* Buffer for holding the certificate ID. */
    char pcCertificateId[ fpdemoCERT_ID_BUFFER_LENGTH ];
    size_t xCertificateIdLength;
    /* Buffer for holding the certificate ownership token. */
    char pcOwnershipToken[ fpdemoOWNERSHIP_TOKEN_BUFFER_LENGTH ];
    size_t xOwnershipTokenLength;
    bool xConnectionEstablished = false;
    CK_SESSION_HANDLE xP11Session;
    uint32_t ulDemoRunCount = 0U;
    CK_RV xPkcs11Ret = CKR_OK;
    uint32_t ulIPAddress = RESET_VALUE;
    char cBuffer[16] = { RESET_VALUE };
    CK_OBJECT_HANDLE certHandle = 0;
    CK_OBJECT_HANDLE pkHandle = 0;


    /* Wait notification from other task */
    bt_status = xTaskNotifyWait(pdFALSE, pdFALSE, &g_console_status, portMAX_DELAY);

	/* version get API for FLEX pack information */
	R_FSP_VersionGet(&version);

	/* Example Project information printed on the RTT */
	APP_PRINT (BANNER_INFO, AP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);

	lfs_err = config_littlFs_flash();
	if (LFS_ERR_OK != lfs_err)
	{
		FAILURE_INDICATION;
		APP_ERR_PRINT("** littleFs flash config failed **\r\n");
        APP_ERR_TRAP(lfs_err);
	}

	/* Initialize the crypto hardware acceleration. */
	 ierr = mbedtls_platform_setup(NULL);
	if (FSP_SUCCESS != ierr)
	{
		APP_ERR_PRINT("** HW SCE Init failed **\r\n");
        APP_ERR_TRAP(ierr);
	}

	/* Prints the Ethernet Configuration header prior to the IP Init. and prints the
	 * Ethernet and IP configuration prior to IP Init. */
	APP_PRINT(ETH_PREINIT);
	print_ipconfig();

	/* Network initialization: FreeRTOS IP Initialization; This initializes the IP stack  */
	bt_status = network_ip_init();
	if(pdFALSE == bt_status)
	{
	   FAILURE_INDICATION;
	   APP_ERR_PRINT("User Network Initialization Failed\r\n");
	   APP_ERR_TRAP(bt_status);
	}

	APP_PRINT("waiting for network link up\r\n");
	bt_status = xTaskNotifyWait(pdFALSE, pdFALSE, &ip_status, portMAX_DELAY);
	if (pdTRUE != bt_status)
	{
	   FAILURE_INDICATION;
	   APP_ERR_PRINT("xTaskNotifyWait Failed\r\n");
        APP_ERR_TRAP(bt_status);
	}

	NETWORK_CONNECT_INDICATION;
	/* Prints the Ethernet Configuration Header for Post IP Init*/
	APP_PRINT(ETH_POSTINIT);
	/* Get the IP address for the MQTT END POINT used for the application*/
	ulIPAddress = FreeRTOS_gethostbyname((char*)democonfigMQTT_BROKER_ENDPOINT);

	if( RESET_VALUE == ulIPAddress )
	{
		FAILURE_INDICATION;
        APP_ERR_PRINT("FreeRTOS_gethostbyname  Failed to get the End point address for %s\r\n",democonfigMQTT_BROKER_ENDPOINT);
        APP_ERR_TRAP(RESET_VALUE);
	}

	/* Convert the IP address to a string to print on to the console. */
	if(NULL == FreeRTOS_inet_ntoa( ulIPAddress, ( char * ) cBuffer))
	{
		APP_PRINT("\r\nDNS Lookup for \"%s\" is      : %s  \r\n", democonfigMQTT_BROKER_ENDPOINT,  cBuffer);
        APP_ERR_TRAP(FSP_ERR_TIMEOUT);
	}
	/* Prints the Ethernet and IP Configuration post IP Init */
	print_ipconfig();

	APP_PRINT("\r\nDNS Lookup for \"%s\" is      : %s  \r\n", democonfigMQTT_BROKER_ENDPOINT,  cBuffer);
    /* Set the pParams member of the network context with desired transport. */
	xNetworkContext.pxParams = &xTlsTransportParams;

    do
    {
        APP_INFO_PRINT( ( "---------STARTING DEMO---------\r\n" ) );

        /* Initialize the buffer lengths to their max lengths. */
        xCertificateLength = fpdemoCERT_BUFFER_LENGTH;
        xCertificateIdLength = fpdemoCERT_ID_BUFFER_LENGTH;
        xOwnershipTokenLength = fpdemoOWNERSHIP_TOKEN_BUFFER_LENGTH;

        /* Initialize the PKCS #11 module */
		xInitializePkcs11Session( &xP11Session );
		xPkcs11Ret = xDestroyDefaultCryptoObjects( xP11Session );

        memcpy(claimKeyPem, (char * )democonfigCLAIM_KEY_PEM, strlen(democonfigCLAIM_KEY_PEM));
        memcpy(claimCertificatePem, (char * )democonfigCLAIM_CERTIFICATE_PEM, strlen(democonfigCLAIM_CERTIFICATE_PEM));


		if( xPkcs11Ret == CKR_OK )
		{
            /* Provision Claim Private Key */
            xPkcs11Ret = xProvisionPrivateKey( xP11Session,
                                               (unsigned char *)claimKeyPem,
											   1 + strlen(claimKeyPem),
                                               ( uint8_t * ) pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
                                               &pkHandle );
		}

        if( xPkcs11Ret == CKR_OK )
        {
            /* Provision Claim Certificate */
            xPkcs11Ret = xProvisionCertificate( xP11Session,
                                                (unsigned char *)claimCertificatePem,
												1 + strlen(claimCertificatePem),
                                               ( uint8_t * ) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
                                               &certHandle );
        }

        /**** Connect to AWS IoT Core with provisioning claim credentials *****/
		/* We first use the claim credentials to connect to the broker. These
		 * credentials should allow use of the RegisterThing API and one of the
		 * CreateCertificatefromCsr or CreateKeysAndCertificate.
		 * In this demo we use CreateCertificatefromCsr. */
		if( xPkcs11Ret == CKR_OK )
		{
			/* Attempts to connect to the AWS IoT MQTT broker. If the
			 * connection fails, retries after a timeout. Timeout value will
			 * exponentially increase until maximum attempts are reached. */
			APP_INFO_PRINT( ( "Establishing MQTT session with claim certificate...\r\n" ) );
			xStatus = xEstablishMqttSession( &xMqttContext,
											 &xNetworkContext,
											 &xBuffer,
											 prvProvisioningPublishCallback,
											 pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS ,
                                             pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS);

			if( xStatus == false )
			{
				APP_ERR_PRINT( ( "Failed to establish MQTT session.\r\n" ) );
			}
			else
			{
				APP_INFO_PRINT( ( "Established connection with claim credentials.\r\n" ) );
				xConnectionEstablished = true;
			}
		}


        /**** Create CSR ***************************/
		xPkcs11Ret = xDestroyDefaultCryptoObjects( xP11Session );

		if( xPkcs11Ret == CKR_OK )
		{
			xStatus = xGenerateKeyAndCsr( xP11Session,
										  pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
										  pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS,
										  pcCsr,
										  fpdemoCSR_BUFFER_LENGTH,
										  &xCsrLength );

			if( xStatus == false )
			{
				APP_ERR_PRINT( ( "Failed to generate Key and Certificate Signing Request.\r\n" ) );
			}
		}



        /* We use the CreateCertificatefromCsr API to obtain a client certificate
         * for a key on the device by means of sending a certificate signing
         * request (CSR). */
        if( xStatus == true )
        {
            /* Subscribe to the CreateCertificateFromCsr accepted and rejected
             * topics. In this demo we use CBOR encoding for the payloads,
             * so we use the CBOR variants of the topics. */
            xStatus = prvSubscribeToCsrResponseTopics();

            if( xStatus == true )
            {
                /* Subscribe to the RegisterThing response topics. */
                xStatus = prvSubscribeToRegisterThingResponseTopics();
            }
        }

        if( xStatus == true )
        {
            /* Create the request payload containing the CSR to publish to the
             * CreateCertificateFromCsr APIs. */
            xStatus = xGenerateCsrRequest( pucPayloadBuffer,
                                           democonfigNETWORK_BUFFER_SIZE,
                                           pcCsr,
                                           xCsrLength,
                                           &xPayloadLength );
        }

        if( xStatus == true )
        {
            /* Publish the CSR to the CreateCertificatefromCsr API. */
            xPublishToTopic( &xMqttContext,
                             FP_CBOR_CREATE_CERT_PUBLISH_TOPIC,
                             FP_CBOR_CREATE_CERT_PUBLISH_LENGTH,
                             ( char * ) pucPayloadBuffer,
                             xPayloadLength );

            if( xStatus == false )
            {
                APP_ERR_PRINT( ( "Failed to publish to fleet provisioning topic: %.*s.\r\n"),
                            FP_CBOR_CREATE_CERT_PUBLISH_LENGTH,
                            FP_CBOR_CREATE_CERT_PUBLISH_TOPIC );
            }
        }

        if( xStatus == true )
        {
            /* From the response, extract the certificate, certificate ID, and
             * certificate ownership token. */
            xStatus = xParseCsrResponse( pucPayloadBuffer,
                                         xPayloadLength,
                                         pcCertificate,
                                         &xCertificateLength,
                                         pcCertificateId,
                                         &xCertificateIdLength,
                                         pcOwnershipToken,
                                         &xOwnershipTokenLength );

            if( xStatus == true )
            {
                APP_INFO_PRINT( ( "Received certificate with Id: %.*s \r\n"), ( int ) xCertificateIdLength, pcCertificateId );
            }
        }

        if( xStatus == true )
        {
            /* Save the certificate into PKCS #11. */
            xStatus = xLoadCertificate( xP11Session,
                                        pcCertificate,
                                        pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
                                        xCertificateLength );

            xPkcs11CloseSession( xP11Session );
        }

        if( xStatus == true )
        {
            /* Unsubscribe from the CreateCertificateFromCsr topics. */
            xStatus = prvUnsubscribeFromCsrResponseTopics();
        }

        /**** Call the RegisterThing API **************************************/

        /* We then use the RegisterThing API to activate the received certificate,
         * provision AWS IoT resources according to the provisioning template, and
         * receive device configuration. */
        if( xStatus == true )
        {
            /* Create the request payload to publish to the RegisterThing API. */
            xStatus = xGenerateRegisterThingRequest( pucPayloadBuffer,
                                                     democonfigNETWORK_BUFFER_SIZE,
                                                     pcOwnershipToken,
                                                     xOwnershipTokenLength,
                                                     democonfigFP_DEMO_ID,
                                                     fpdemoFP_DEMO_ID_LENGTH,
                                                     &xPayloadLength );
        }

        if( xStatus == true )
        {
            /* Publish the RegisterThing request. */
            xPublishToTopic( &xMqttContext,
                             FP_CBOR_REGISTER_PUBLISH_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ),
                             FP_CBOR_REGISTER_PUBLISH_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                             ( char * ) pucPayloadBuffer,
                             xPayloadLength );

            if( xStatus == false )
            {
                APP_ERR_PRINT( ( "Failed to publish to fleet provisioning topic: %.*s.\r\n"),
                            FP_CBOR_REGISTER_PUBLISH_LENGTH( fpdemoPROVISIONING_TEMPLATE_NAME_LENGTH ),
                            FP_CBOR_REGISTER_PUBLISH_TOPIC( democonfigPROVISIONING_TEMPLATE_NAME ) );
            }
        }

        if( xStatus == true )
        {
            /* Extract the Thing name from the response. */
            xThingNameLength = fpdemoMAX_THING_NAME_LENGTH;
            xStatus = xParseRegisterThingResponse( pucPayloadBuffer,
                                                   xPayloadLength,
                                                   pcThingName,
                                                   &xThingNameLength );

            if( xStatus == true )
            {
                APP_INFO_PRINT( ( "Received AWS IoT Thing name: %.*s\r\n"), ( int ) xThingNameLength, pcThingName );
            }
        }

        if( xStatus == true )
        {
            /* Unsubscribe from the RegisterThing topics. */
            prvUnsubscribeFromRegisterThingResponseTopics();
        }

        /**** Disconnect from AWS IoT Core ************************************/

        /* As we have completed the provisioning workflow, we disconnect from
         * the connection using the provisioning claim credentials. We will
         * establish a new MQTT connection with the newly provisioned
         * credentials. */
        if( xConnectionEstablished == true )
        {
            xDisconnectMqttSession( &xMqttContext, &xNetworkContext );
            xConnectionEstablished = false;
        }

        /**** Connect to AWS IoT Core with provisioned certificate ************/

        if( xStatus == true )
        {
            APP_INFO_PRINT( ( "Establishing MQTT session with provisioned certificate...\r\n" ) );
            xStatus = xEstablishMqttSession( &xMqttContext,
                                             &xNetworkContext,
                                             &xBuffer,
                                             prvProvisioningPublishCallback,
                                             pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
                                             pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS );

            if( xStatus != true )
            {
                APP_ERR_PRINT( ( "Failed to establish MQTT session with provisioned "
                            "credentials. Verify on your AWS account that the "
                            "new certificate is active and has an attached IoT "
                            "Policy that allows the \"iot:Connect\" action.\r\n" ) );
            }
            else
            {
                APP_INFO_PRINT( ( "Successfully established connection with provisioned credentials.\r\n" ) );
                xConnectionEstablished = true;
            }
        }

        /**** Finish **********************************************************/

        if( xConnectionEstablished == true )
        {
            /* Close the connection. */
            xDisconnectMqttSession( &xMqttContext, &xNetworkContext );
            xConnectionEstablished = false;
        }

        /**** Retry in case of failure ****************************************/

        /* Increment the demo run count. */
        ulDemoRunCount++;

        if( xStatus == true )
        {
            //APP_INFO_PRINT( ( "Demo iteration %d is successful.", ulDemoRunCount ) );
        }
        /* Attempt to retry a failed iteration of demo for up to #fpdemoMAX_DEMO_LOOP_COUNT times. */
        else if( ulDemoRunCount < fpdemoMAX_DEMO_LOOP_COUNT )
        {
            //APP_WARN_PRINT( ( "Demo iteration %d failed. Retrying...", ulDemoRunCount ) );
            vTaskDelay( fpdemoDELAY_BETWEEN_DEMO_RETRY_ITERATIONS_SECONDS );
        }
        /* Failed all #fpdemoMAX_DEMO_LOOP_COUNT demo iterations. */
        else
        {
            //APP_ERR_PRINT( ( "All %d demo iterations failed.", fpdemoMAX_DEMO_LOOP_COUNT ) );
            break;
        }
    } while( xStatus != true );

    /* Log demo success. */
    if( xStatus == true )
    {
        APP_INFO_PRINT( ( "Demo completed successfully." ) );
        APP_INFO_PRINT( ( "-------DEMO FINISHED-------\r\n" ) );
    }

    /* Delete this task. */
    APP_INFO_PRINT( ( "Deleting Fleet Provisioning Demo task.\r\n" ) );
    vTaskDelete( NULL );

    //return ( xStatus == true ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*-----------------------------------------------------------*/
