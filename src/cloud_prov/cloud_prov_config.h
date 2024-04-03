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

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for DEMO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for DEMO.
 */

#include "logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "FLEET_PROVISIONING_DEMO"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

/* Prototype for the function used to print to console on Windows simulator
 * of FreeRTOS.
 * The function prints to the console before the network is connected;
 * then a UDP port after the network has connected. */
extern void vLoggingPrintf( const char * pcFormatString,
                            ... );

/* Map the SdkLog macro to the logging function to enable logging
 * on Windows simulator. */
#ifndef SdkLog
    #define SdkLog( message )    vLoggingPrintf message
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/

/**
 * @brief The unique ID used by the demo to differentiate instances.
 *
 *!!! Please note a #defined constant is used for convenience of demonstration
 *!!! only.  Production devices can use something unique to the device that can
 *!!! be read by software, such as a production serial number, instead of a
 *!!! hard coded constant.
 */
#define democonfigFP_DEMO_ID    "FPDemoID"__TIME__

/**
 * @brief The MQTT client identifier used in this example.  Each client identifier
 * must be unique so edit as required to ensure no two clients connecting to the
 * same broker use the same client identifier.
 *
 * @note Appending __TIME__ to the client id string will reduce the possibility of a
 * client id collision in the broker. Note that the appended time is the compilation
 * time. This client id can cause collision, if more than one instance of the same
 * binary is used at the same time to connect to the broker.
 */
#ifndef democonfigCLIENT_IDENTIFIER
    #define democonfigCLIENT_IDENTIFIER    "client"democonfigFP_DEMO_ID
#endif

/**
 * @brief Details of the MQTT broker to connect to.
 *
 * This is the Claim's Rest API Endpoint for AWS IoT.
 *
 * @note Your AWS IoT Core endpoint can be found in the AWS IoT console under
 * Settings/Custom Endpoint, or using the describe-endpoint API.
 *
 * #define CLOUD_PROV_DEV_MQTT_BROKER_ENDPOINT     "...insert here..."
 */
#define CLOUD_PROV_DEV_MQTT_BROKER_ENDPOINT     "a2t8kq9jnlwzum-ats.iot.us-east-1.amazonaws.com"

/**
 * @brief AWS IoT MQTT broker port number.
 *
 * In general, port 8883 is for secured MQTT connections.
 *
 * @note Port 443 requires use of the ALPN TLS extension with the ALPN protocol
 * name. When using port 8883, ALPN is not required.
 */
#define CLOUD_PROV_MQTT_BROKER_PORT    ( 8883 )

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define CLOUD_PROV_MQTT_SEND_RECV_TIMEOUT_MS    ( 200U )

/**
 * @brief The maximum number of retries for network operation with server.
 */
#define CLOUD_PROV_TLS_RETRY_MAX_ATTEMPTS                           ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server.
 */
#define CLOUD_PROV_TLS_MAX_BACKOFF_DELAY_MS                   ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts.
 */
#define CLOUD_PROV_TLS_RETRY_BACKOFF_BASE_MS                        ( 500U )

/**
 * @brief Keep alive time reported to the broker while establishing an MQTT connection.
 *
 * It is the responsibility of the Client to ensure that the interval between
 * Control Packets being sent does not exceed this Keep Alive value. In the
 * absence of sending any other Control Packets, the Client MUST send a
 * PINGREQ Packet.
 */
#define CLOUD_PROV_MQTT_KEEP_ALIVE_TIMEOUT_SEC        ( 60U )

/**
 * @brief Timeout for receiving MQTT's CONNACK packet in milliseconds.
 */
#define CLOUD_PROV_MQTT_CONNACK_RECV_TIMEOUT_MS           ( 10000U )

/**
 * @brief Size of the network buffer for MQTT packets. Must be large enough to
 * hold the GetCertificateFromCsr response, which, among other things, includes
 * a PEM encoded certificate.
 */
#define CLOUD_PROV_MQTT_BUFFER_SIZE       ( 2048U )

/**
 * @brief Server's root CA certificate.
 *
 * For AWS IoT MQTT broker, this certificate is used to identify the AWS IoT
 * server and is publicly available. Refer to the AWS documentation available
 * in the link below.
 * https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs
 *
 * @note This certificate should be PEM-encoded.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 *
 * #define CLOUD_PROV_DEV_ROOT_CA_PEM    "...insert here..."
 */
/* ECDSA claim credentials  */
// TODO Remove this when done debugging RSA vs ECDSA claim credentials
/*#define CLOUD_PROV_DEV_ROOT_CA_PEM 	    "-----BEGIN CERTIFICATE-----\n" \
	    "MIIB5TCCAYugAwIBAgIUY54uCK5sqRGXqBEio8xNkp0GS4YwCgYIKoZIzj0EAwIw\n" \
	    "SDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExEjAQBgNVBAcMCVNh\n" \
	    "biBEaWVnbzEQMA4GA1UECgwHUmVuZXNhczAeFw0yNDAzMTkyMDQ1MjNaFw0yNDA0\n" \
	    "MTgyMDQ1MjNaMEgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMRIw\n" \
	    "EAYDVQQHDAlTYW4gRGllZ28xEDAOBgNVBAoMB1JlbmVzYXMwWTATBgcqhkjOPQIB\n" \
	    "BggqhkjOPQMBBwNCAATXPSSgl6h+gP2/5Uop+4oKJNmHwR6/6TZSGmzJl3c+F0vm\n" \
	    "OhFbPIf5Jj/sDWKgCvMeXIZrrP9GdLmXnI4Va5ouo1MwUTAdBgNVHQ4EFgQUG539\n" \
	    "fU814Ype+hY9LxOnyY3zuGYwHwYDVR0jBBgwFoAUG539fU814Ype+hY9LxOnyY3z\n" \
	    "uGYwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiACwGTyLjH7wzVy\n" \
	    "W7QqCGPVN/HDz8aGzlMb6EmcBvT5bQIhAK0KwriQ5Pu7MvkvB8i/FPdQQBj2cELY\n" \
        "MeQ+Maet5aTi\n" \
	    "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_CERT_PEM 	    "-----BEGIN CERTIFICATE-----\n" \
        "MIIB+DCCAZ+gAwIBAgIUIJNeFY8sJCvZMCD6PyzvddXuBt4wCgYIKoZIzj0EAwIw\n" \
        "SDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExEjAQBgNVBAcMCVNh\n" \
        "biBEaWVnbzEQMA4GA1UECgwHUmVuZXNhczAeFw0yNDAzMTkyMTA1MDVaFw0yNDA0\n" \
        "MDIyMTA1MDVaMFwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMRIw\n" \
        "EAYDVQQHDAlTYW4gRGllZ28xEDAOBgNVBAoMB1JlbmVzYXMxEjAQBgNVBAsMCU1h\n" \
        "cmtldGluZzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABBXZ0SFUUx0/2mnA0yMw\n" \
        "rGenxEq+rCws+sg521Hhvdh7fi6LyOdaqx6WgK9wmoTGwf51/6hUglg+ECjqTjqf\n" \
        "WEajUzBRMB0GA1UdDgQWBBR4OfjXZYB3hOLCn8fkOL/lei07aDAfBgNVHSMEGDAW\n" \
        "gBQbnf19TzXhil76Fj0vE6fJjfO4ZjAPBgNVHRMBAf8EBTADAQH/MAoGCCqGSM49\n" \
        "BAMCA0cAMEQCIGwPoWpA08dmsFKrLLR05TeL/4Yg4WnB3GHW1ut8YCeDAiAHW+wW\n" \
        "fvcoGSUSz+Y2gI3NJbf2F+pfi8C67SsIl54sMw==\n" \
        "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_KEY_PEM 	    "-----BEGIN PRIVATE KEY-----\n" \
        "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQg91HlJoiS/iPtEbar\n" \
        "ftJWCN1PE523iEHQQEFsf0Paua+hRANCAAQV2dEhVFMdP9ppwNMjMKxnp8RKvqws\n" \
        "LPrIOdtR4b3Ye34ui8jnWqseloCvcJqExsH+df+oVIJYPhAo6k46n1hG\n" \
        "-----END PRIVATE KEY-----"*/

// TODO Remove this when done debugging RSA vs ECDSA claim credentials
/* Jyotish RSA claim credentials */
/*#define CLOUD_PROV_DEV_ROOT_CA_PEM 	    "-----BEGIN CERTIFICATE-----\n" \
	    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
	    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
	    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
	    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
	    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
	    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
	    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
	    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
	    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
	    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
	    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
	    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
	    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
	    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
	    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
	    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
	    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
	    "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
	    "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_CERT_PEM         "-----BEGIN CERTIFICATE-----\n" \
        "MIIDWjCCAkKgAwIBAgIVALkf8KQpDz8OsUOhvb6hW4OeXwpxMA0GCSqGSIb3DQEB\n" \
        "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
        "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDAyMjYwMTU0\n" \
        "MjJaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
        "dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDGwi0/5JwFIHh7WZ3E\n" \
        "XZyzIDG1rYdLGW9aftLk4Nvi60ndAulVZL9WOpMqqZen/f8jOAIUJpZRurKYFb+Z\n" \
        "0oZ4AazrHyr7rTnaZVJmBIB/hEPfhMMTGSE8adeFF03Bf0XxWMWCVD+J0ClT12z0\n" \
        "zxoo92CZEuxM0oHHX5Ca7bPXST+R1e3EOSk4Xs0i3QsMNwgPJBkFaX4mELUQuyFT\n" \
        "ketuZQdLxBKKvU4TOFFQlpa81jP09mjTsjSyVGMFPv6AD11KjstggG7gePHL8iWZ\n" \
        "/FuqmxEsUSf446U/E1dmbkNJEpD/G6J90iJF+5WTVvmyQ9xf61SBHHps5trLzo2L\n" \
        "MFo1AgMBAAGjYDBeMB8GA1UdIwQYMBaAFFt7NJHXlcb8MV/6oRs6T0Q4NC/ZMB0G\n" \
        "A1UdDgQWBBSSBqSFIhEQGhb7JsEBadJfVLA0EDAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
        "DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAl8Tk7yAH9jWujtpXFIeYNoCH\n" \
        "0qezhwlzk4pCRKxRpb89eoLt9XMY1VhiU83aBD7xHSX/9Q5tMPAusl7ZjyDZqYeI\n" \
        "MNOr4cmWHVPZOYi8VLzlqFLoCJzoCbH+48u236WCcV2F5oLNM1405dZgmgUxHon8\n" \
        "Brku69X5fncKnthKufwfw1llou/NTyZBdQsCLBaRST+qsZ58lN7ejZwXyZrIfa4F\n" \
        "bz7zIK/PSii0TgynQXvq8L2unPW8Tx1TqwYH7QFQ82o0kQ6ZIcRC1NzbvQUinIP6\n" \
        "y6OmXO+N/EpuVVXHgyGHA80R2Z3+Dx7EKbQ5nZjczC6PLKb0mwtPqZk4hARehQ==\n" \
        "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_KEY_PEM         "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEpAIBAAKCAQEAxsItP+ScBSB4e1mdxF2csyAxta2HSxlvWn7S5ODb4utJ3QLp\n" \
        "VWS/VjqTKqmXp/3/IzgCFCaWUbqymBW/mdKGeAGs6x8q+6052mVSZgSAf4RD34TD\n" \
        "ExkhPGnXhRdNwX9F8VjFglQ/idApU9ds9M8aKPdgmRLsTNKBx1+Qmu2z10k/kdXt\n" \
        "xDkpOF7NIt0LDDcIDyQZBWl+JhC1ELshU5HrbmUHS8QSir1OEzhRUJaWvNYz9PZo\n" \
        "07I0slRjBT7+gA9dSo7LYIBu4Hjxy/IlmfxbqpsRLFEn+OOlPxNXZm5DSRKQ/xui\n" \
        "fdIiRfuVk1b5skPcX+tUgRx6bObay86NizBaNQIDAQABAoIBAAWOWAV5NoNOLRAF\n" \
        "qUtb8o1vUPRrLWUECQDWmr6bKoplWWM8OZmRf2fBq2t94idoCkHJxwUZLwqJ1YQ6\n" \
        "12hNYkTohxTrgiAW748TVgJUDeOCpwMQiwN1qGAv9T5bMGlATYw5lwvVnxETKJSX\n" \
        "aQxO3cRXZfPhe4mKOOaB3WD3VBeof+eUujJx8OzKup8xBZpIoaQlVm75TyMCcT8r\n" \
        "A/0ItJUSl3yBMWWJKbSeRZJKIBKXBpMMTQW95xbV3Ya/En/eDufw2n0LKi8gIECG\n" \
        "ZHlEwkpxTkSsnhXJgK7nU7XMQj1AB4YXNiE0HAPNXYTXrpC038yqI/u58zXZ0Yex\n" \
        "q1Sho+ECgYEA6lvGv0zRfaNPRpIfycl3JsYu7aPMMNqRqSTqE+/gGNpHOv6FPnBG\n" \
        "xZYVexB/uYnF8bTGDEmNO+krfCwf/vxyEY0JXqUSmGGVGNWmmu9jjK2m69yLE7OC\n" \
        "DA4I1gHzgqtqVS8AqdvG7VrWrljTVg77q2kLPCclwNTIyQm7le6sC2cCgYEA2RzR\n" \
        "csYyqGfVegAFP65QqLjkClwPl5dLBzl5aiskvaYeR2W+kBbAa7u962l+f8EpztEd\n" \
        "WOaFjew9iEy9kP1jVhTAVYzOFeVqFTzpleSOQAIKZ4za7wtOzstPc0LyXOlbjJCP\n" \
        "wuplBxjJDO3FHo2PqpnReMoK/u6ceqE1MxkHCAMCgYEAlWWgEv8zZLYgqUopfYcy\n" \
        "r8MS02bhmhsbVAo2NjNqVi1/zvnFkwIb+4UreGISKgLL7sNgpSCWKiUAFY5Db1ca\n" \
        "mFmiKXVtnzpFw6kfJhGJEnr4t87F0e9S7cBcnaBszVXc2SS3dZCnBVQtGsOBJZEz\n" \
        "mhfCk7wY8w4yWQYdUPzvK+0CgYEAlv+f+/80hEOTCUKyY9PMbUwJ7dqRTZD/sYqb\n" \
        "kudmqi+6p1Vv766jLUppkCzSue+SMDDoBEhvYoGHWiBlR02zpM7R5a4ENt5TpWmz\n" \
        "23gWJxZEPjiMm+x1ZwWnwDYzccMq1NxF3/49PW5ThhZoHhO+c5x/P07lzuANHIy5\n" \
        "/0vMxBkCgYB6YIrqV5ZM/iXVhwnXlWVgZ2ucKWGyyACoH/NwSaaqr+bRo0n3jo0l\n" \
        "gB7qU4Fn+0XN+ldXzLSayNeTxFSQMTsqJa4mdaS+rVEdbHZa1stqukC7OqoO8moi\n" \
        "4d+TBNoyoA8EuODEth84dwy+qM5zwgzw+K/lutZNhBDZEyViKuqHGw==\n" \
        "-----END RSA PRIVATE KEY-----"*/

// TODO Remove this when done debugging RSA vs ECDSA claim credentials
/* RSA claim credentials  */
#define CLOUD_PROV_DEV_ROOT_CA_PEM 	    "-----BEGIN CERTIFICATE-----\n" \
	    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
        "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
        "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
        "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
        "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
        "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
        "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
        "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
        "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
        "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
        "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
        "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
        "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
        "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
        "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
        "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
        "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
        "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
	    "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_CERT_PEM 	    "-----BEGIN CERTIFICATE-----\n" \
        "MIIDWTCCAkGgAwIBAgIURhZFiuVl2hZznpv3kjrngTKBOv4wDQYJKoZIhvcNAQEL\n" \
        "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
        "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI0MDMyMDAxMTI1\n" \
        "MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
        "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANapxCiDNfCpU+yp7txL\n" \
        "gfH0pU035cCVr6IIszkNWO/f/J3NyJ00Nd2mrZMlIMYCkea1icCvWiBW3tWbBThd\n" \
        "sv9X1WL+5XO2r7VSJDurYWlN+j9TjcVtDL1MaNVmxOxG+dgEbWrvYi5H2ojn9zV1\n" \
        "L9f+ZOickOP/3KgcBatR6blSk6ZU4YNgtNWC1CoXyY00ToYHnJ3VrwPbV6pDUh4Y\n" \
        "SFOiCxvbcYkgObFGQqAtcaPkYlu5ARHRGhZdtlgskVYI5ztVrtdoWbGWfnN6MM02\n" \
        "kyRRvuwOWovTY1TqrAWv+yEsHg3tFl7TmWIQqlwyv0tGVbz995q/JykS8uo/IFEZ\n" \
        "CskCAwEAAaNgMF4wHwYDVR0jBBgwFoAUA8lvqaoUcIVKjIi3M7WOWSX3DcQwHQYD\n" \
        "VR0OBBYEFEXjVr1V/FjkLYt1K7RRGQaD+KL0MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
        "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBJ70F1wpecdu+hWeIEtndom8iW\n" \
        "GHNQLNka19w2G4N9apX5D6chTObzMox9TE7DMUF+b+5rvhTLnkmY4lcf3NE/ASg7\n" \
        "G4/XgQgz9DSA79Ry5/5uE9fAxs8GugLQXeeJO3PQNzOf51FU0gBmTTjFJv93sfT9\n" \
        "cOrTczXHwEfyJTqkX/YKsLmy9JxTmJ6reeeiOBKvrhhJAEvTo0+nRk37Kmurw/Zs\n" \
        "oXKFtTMQipVU+SlkGaSOGhSBM9ArxlzY+VNK2EsVgZEUvkry0cymSDvKsrxhobtM\n" \
        "tYZLSLqJShSahFcvtYZvTjikCGfED9/D7uVFGdNEJ9gBl8qDK/J5oQC288Eo\n" \
        "-----END CERTIFICATE-----"

#define CLOUD_PROV_DEV_CLAIM_KEY_PEM 	 "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEogIBAAKCAQEA1qnEKIM18KlT7Knu3EuB8fSlTTflwJWvogizOQ1Y79/8nc3I\n" \
        "nTQ13aatkyUgxgKR5rWJwK9aIFbe1ZsFOF2y/1fVYv7lc7avtVIkO6thaU36P1ON\n" \
        "xW0MvUxo1WbE7Eb52ARtau9iLkfaiOf3NXUv1/5k6JyQ4//cqBwFq1HpuVKTplTh\n" \
        "g2C01YLUKhfJjTROhgecndWvA9tXqkNSHhhIU6ILG9txiSA5sUZCoC1xo+RiW7kB\n" \
        "EdEaFl22WCyRVgjnO1Wu12hZsZZ+c3owzTaTJFG+7A5ai9NjVOqsBa/7ISweDe0W\n" \
        "XtOZYhCqXDK/S0ZVvP33mr8nKRLy6j8gURkKyQIDAQABAoIBABQSa7UeD70hGCfY\n" \
        "++y9YrktFwfRYjpPxMxx49BB3z8OwqA+zbNlXmQLYc+QeTAKrXnVH7xUqSi89rki\n" \
        "dap/mG0n2VwYWXwoaOWbeKfbrpAaKnwhY/BB8h8BNgw+P8uYF2SLhW+qUpPhRB3p\n" \
        "EsR84KxhrS3S9flbw1Y0yJLsZd5eHmRsqyNfIUSQmtT3HNzMt3YZ1BHSDR9jkS1d\n" \
        "4IAX5Bk+fAw6M+mehuPPUU1h4jh0DknrGqlNe3Apd/Q7z+BTX7dJXWFwLLwgo9Ki\n" \
        "32kzBP2nF1NU/krtql0AaTep8xN2OVrh+MsqqxbMatStvAvtQJrmb209E23SuAJw\n" \
        "p/mZ4AECgYEA+Sg+pJEliABBAW8YgRFiz8bAxrL6Mqp2KCjJo4ORW77HKPkLjTBJ\n" \
        "8WcMeLLdpMkd1SgQcVaKW2rYBBtLPA3vgBEWbwBDhN7vxcpnhacggvPoFHgHp69B\n" \
        "8hmNjKsy1P4loLoxD8Mhwhxc6gQxnt3LFCERyall+Ewp5qTe2ABQgokCgYEA3I8A\n" \
        "3gfcEUYjxSEuqeheWd34NQNVLhXbeFK58DE8jkniruO4fevI5JmBw2F7Q2Y15NkH\n" \
        "8879BZ6eFphLcC3lQ/ZmfY0Ng25/9K4j9iyrMZTWaIONpipFsPQP/u6+NEgRUSp9\n" \
        "Crz4bkVjeXJiA/NxYAPmZpqZ/Z0nonJ3PdxnNkECgYBEscDbtJ0JazCBCbVS2I1K\n" \
        "xfyei/gJghADsIMvjvXDuszEUi3KJpcsdIHuwTn01fJgs6Qc64RIAiQ+tZu/mgP/\n" \
        "ZjauMf9rod+vw8dZBSQklsTU5+cMZ3iiHEN3ljf0m4GaV9txAhbLovBHIRLhPztf\n" \
        "eGKkHA1eMPmYerpqCWQpKQKBgD0Xm0zk7m8Wd4f99ff4NlI8Kqk8gBag1UbR21EN\n" \
        "wlFDY/e/efXfHqk7X/cnhEYaBKmb+BjvZ6Rr2Cw4CQVMoSmfEaqLLIgQTn8O0UU6\n" \
        "AD1elbkjX1P8d4tZPykFJkhDAhDybOpRh5q7u58DMOUZRHks+1/GSQs3wae8fwzy\n" \
        "hOBBAoGAMzpTqxzpY8BUPSlhm9Iszov4+MyVrPF6jr09KMBqssSRZ5X7aGjGEjH6\n" \
        "jEBfmzL5A8h1jJNaq//1yYXyGynLTBg9M4hHLIqpIYi2fZA+nZSt/edr0NrdgwY+\n" \
        "3klo1T5mZiuNNxK6gVmC6pE37xgN0VDpryaMsly68AemeQ60btA=\n" \
        "-----END RSA PRIVATE KEY-----"

/**
 * @brief Name of the provisioning template to use for the RegisterThing
 * portion of the Fleet Provisioning workflow.
 *
 * @details For information about provisioning templates, see the following AWS documentation:
 * https://docs.aws.amazon.com/iot/latest/developerguide/provision-template.html#fleet-provision-template

 *
 * @note The provisioning template MUST be created in AWS IoT before running the
 * demo.
 *
 * @note If you followed the manual setup steps on https://freertos.org/iot-fleet-provisioning/demo.html,
 * the provisioning template name is "FleetProvisioningDemoTemplate".
 * However, if you used different name for you setup, you should change this macro
 * to fit the AWS provisioning template
 */
#define CLOUD_PROV_TEMPLATE_NAME    "FleetProvisioningTemplate"

/**
 * @brief The length of #CLOUD_PROV_TEMPLATE_NAME.
 */
#define CLOUD_PROV_TEMPLATE_NAME_LENGTH    ( ( uint16_t ) ( sizeof( CLOUD_PROV_TEMPLATE_NAME ) - 1 ) )

/**
 * @brief Subject name to use when creating the certificate signing request (CSR)
 * for provisioning the demo client with using the Fleet Provisioning
 * CreateCertificateFromCsr APIs.
 *
 * This is passed to MbedTLS; see https://tls.mbed.org/api/x509__csr_8h.html#a954eae166b125cea2115b7db8c896e90
 */
#ifndef democonfigCSR_SUBJECT_NAME
    #define democonfigCSR_SUBJECT_NAME    "CN="democonfigFP_DEMO_ID
#endif

/**
 * @brief Set the stack size of the main demo task.
 *
 * In the Windows port, this stack only holds a structure. The actual
 * stack is created by an operating system thread.
 *
 * @note This demo runs on WinSim and the minimal stack size is functional.
 * However, if you are porting components of this demo to other platforms,
 * the stack size may need to be increased to accommodate the size of the
 * buffers used when generating new keys and certificates.
 *
 */
#define democonfigDEMO_STACKSIZE            configMINIMAL_STACK_SIZE



/**
 * @brief The name of the operating system that the application is running on.
 * The current value is given as an example. Please update for your specific
 * operating system.
 */
#define democonfigOS_NAME                   "FreeRTOS"

/**
 * @brief The version of the operating system that the application is running
 * on. The current value is given as an example. Please update for your specific
 * operating system version.
 */
#define democonfigOS_VERSION                tskKERNEL_VERSION_NUMBER

/**
 * @brief The name of the hardware platform the application is running on. The
 * current value is given as an example. Please update for your specific
 * hardware platform.
 */
#define democonfigHARDWARE_PLATFORM_NAME    "WinSim"

/**
 * @brief The name of the MQTT library used and its version, following an "@"
 * symbol.
 */
#define democonfigMQTT_LIB    "core-mqtt@"MQTT_LIBRARY_VERSION

#endif /* DEMO_CONFIG_H */
