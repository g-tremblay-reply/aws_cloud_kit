//
// Created by Gabriel on 3/23/2024.
//

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

/*************************************************************************************
 *                                  INCLUDES
 ************************************************************************************/
#include <cloud_prov_serializer.h>
#include "cloud_prov_config.h"

/*************************************************************************************
 * DEFINE MACROS
 ************************************************************************************/


/*************************************************************************************
 * Type Definitions
 ************************************************************************************/


/*************************************************************************************
 * Local Variables
 ************************************************************************************/


/*************************************************************************************
 * Local Function Prototypes
 ************************************************************************************/

/*************************************************************************************
 * Local Functions
 ************************************************************************************/


/*************************************************************************************
 * global functions
 ************************************************************************************/

bool CloudProv_SerializeCsr(const uint8_t * csrBuffer,
                            size_t csrLength,
                            size_t payloadBufferSize,
                            uint8_t * payloadBuffer,
                            size_t * payloadLength )
{
    CborEncoder xEncoder, xMapEncoder;
    CborError xCborRet;

    configASSERT(payloadBuffer != NULL );
    configASSERT(csrBuffer != NULL );
    configASSERT(payloadLength != NULL );

    /* For details on the CreateCertificatefromCsr request payload format, see:
     * https://docs.aws.amazon.com/iot/latest/developerguide/fleet-provision-api.html#create-cert-csr-request-payload
     */
    cbor_encoder_init(&xEncoder, payloadBuffer, payloadBufferSize, 0 );

    /* The request document is a map with 1 key value pair. */
    xCborRet = cbor_encoder_create_map( &xEncoder, &xMapEncoder, 1 );

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_stringz( &xMapEncoder, "certificateSigningRequest" );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_string(&xMapEncoder, csrBuffer, csrLength );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encoder_close_container( &xEncoder, &xMapEncoder );
    }

    if( xCborRet == CborNoError )
    {
        *payloadLength = cbor_encoder_get_buffer_size(&xEncoder, ( uint8_t * ) payloadBuffer );
    }
    else
    {
        LogError( ( "Error during CBOR encoding: %s", cbor_error_string( xCborRet ) ) );

        if( ( xCborRet & CborErrorOutOfMemory ) != 0 )
        {
            LogError( ( "Cannot fit CreateCertificateFromCsr request payload into buffer." ) );
        }
    }

    return( xCborRet == CborNoError );
}
/*-----------------------------------------------------------*/

CborError CloudProv_SerializeRegisterThingRequest(const char *certOwnershipToken,
                                             size_t certOwnershipTokenLenght,
                                             const char *serialNb,
                                             size_t serialNbLenght,
                                             size_t payloadBufferSize,
                                             uint8_t *payloadBuffer,
                                             size_t *payloadLength )
{
    CborEncoder xEncoder, xMapEncoder, xParametersEncoder;
    CborError xCborRet;


    /* For details on the RegisterThing request payload format, see:
     * https://docs.aws.amazon.com/iot/latest/developerguide/fleet-provision-api.html#register-thing-request-payload
     */
    cbor_encoder_init( &xEncoder, payloadBuffer, payloadBufferSize, 0 );
    /* The RegisterThing request payload is a map with two keys. */
    xCborRet = cbor_encoder_create_map( &xEncoder, &xMapEncoder, 2 );

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_stringz( &xMapEncoder, "certificateOwnershipToken" );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_string( &xMapEncoder, certOwnershipToken, certOwnershipTokenLenght );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_stringz( &xMapEncoder, "parameters" );
    }

    if( xCborRet == CborNoError )
    {
        /* Parameters in this example is length 1. */
        xCborRet = cbor_encoder_create_map( &xMapEncoder, &xParametersEncoder, 1 );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_stringz( &xParametersEncoder, "SerialNumber" );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encode_text_string( &xParametersEncoder, serialNb, serialNbLenght );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encoder_close_container( &xMapEncoder, &xParametersEncoder );
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_encoder_close_container( &xEncoder, &xMapEncoder );
    }

    if( xCborRet == CborNoError )
    {
        *payloadLength = cbor_encoder_get_buffer_size(&xEncoder, ( uint8_t * ) payloadBuffer );
    }
    else
    {
        LogError( ( "Error during CBOR encoding: %s", cbor_error_string( xCborRet ) ) );

        if( ( xCborRet & CborErrorOutOfMemory ) != 0 )
        {
            LogError( ( "Cannot fit RegisterThing request payload into buffer." ) );
        }
    }

    return xCborRet;
}
/*-----------------------------------------------------------*/

bool CloudProv_DeserializeCsrResponse(const uint8_t * pucResponse,
                                      size_t xLength,
                                      char * pcCertificateBuffer,
                                      size_t * pxCertificateBufferLength,
                                      char * pcCertificateIdBuffer,
                                      size_t * pxCertificateIdBufferLength,
                                      char * pcOwnershipTokenBuffer,
                                      size_t * pxOwnershipTokenBufferLength )
{
    CborError xCborRet;
    CborParser xParser;
    CborValue xMap;
    CborValue xValue;

    configASSERT( pucResponse != NULL );
    configASSERT( pcCertificateBuffer != NULL );
    configASSERT( pxCertificateBufferLength != NULL );
    configASSERT( pcCertificateIdBuffer != NULL );
    configASSERT( pxCertificateIdBufferLength != NULL );
    configASSERT( *pxCertificateIdBufferLength >= 64 );
    configASSERT( pcOwnershipTokenBuffer != NULL );
    configASSERT( pxOwnershipTokenBufferLength != NULL );

    /* For details on the CreateCertificatefromCsr response payload format, see:
     * https://docs.aws.amazon.com/iot/latest/developerguide/fleet-provision-api.html#register-thing-response-payload
     */
    xCborRet = cbor_parser_init( pucResponse, xLength, 0, &xParser, &xMap );

    if( xCborRet != CborNoError )
    {
        LogError( ( "Error initializing parser for CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
    }
    else if( !cbor_value_is_map( &xMap ) )
    {
        LogError( ( "CreateCertificateFromCsr response is not a valid map container type." ) );
    }
    else
    {
        xCborRet = cbor_value_map_find_value( &xMap, "certificatePem", &xValue );

        if( xCborRet != CborNoError )
        {
            LogError( ( "Error searching CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
        }
        else if( xValue.type == CborInvalidType )
        {
            LogError( ( "\"certificatePem\" not found in CreateCertificateFromCsr response." ) );
        }
        else if( xValue.type != CborTextStringType )
        {
            LogError( ( "Value for \"certificatePem\" key in CreateCertificateFromCsr response is not a text string type." ) );
        }
        else
        {
            xCborRet = cbor_value_copy_text_string( &xValue, pcCertificateBuffer, pxCertificateBufferLength, NULL );

            if( xCborRet == CborErrorOutOfMemory )
            {
                size_t requiredLen = 0;
                ( void ) cbor_value_calculate_string_length( &xValue, &requiredLen );
                LogError( ( "Certificate buffer insufficiently large. Certificate length: %lu", ( unsigned long ) requiredLen ) );
            }
            else if( xCborRet != CborNoError )
            {
                LogError( ( "Failed to parse \"certificatePem\" value from CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
            }
        }
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_value_map_find_value( &xMap, "certificateId", &xValue );

        if( xCborRet != CborNoError )
        {
            LogError( ( "Error searching CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
        }
        else if( xValue.type == CborInvalidType )
        {
            LogError( ( "\"certificateId\" not found in CreateCertificateFromCsr response." ) );
        }
        else if( xValue.type != CborTextStringType )
        {
            LogError( ( "\"certificateId\" is an unexpected type in CreateCertificateFromCsr response." ) );
        }
        else
        {
            xCborRet = cbor_value_copy_text_string( &xValue, pcCertificateIdBuffer, pxCertificateIdBufferLength, NULL );

            if( xCborRet == CborErrorOutOfMemory )
            {
                size_t requiredLen = 0;
                ( void ) cbor_value_calculate_string_length( &xValue, &requiredLen );
                LogError( ( "Certificate ID buffer insufficiently large. Certificate ID length: %lu", ( unsigned long ) requiredLen ) );
            }
            else if( xCborRet != CborNoError )
            {
                LogError( ( "Failed to parse \"certificateId\" value from CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
            }
        }
    }

    if( xCborRet == CborNoError )
    {
        xCborRet = cbor_value_map_find_value( &xMap, "certificateOwnershipToken", &xValue );

        if( xCborRet != CborNoError )
        {
            LogError( ( "Error searching CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
        }
        else if( xValue.type == CborInvalidType )
        {
            LogError( ( "\"certificateOwnershipToken\" not found in CreateCertificateFromCsr response." ) );
        }
        else if( xValue.type != CborTextStringType )
        {
            LogError( ( "\"certificateOwnershipToken\" is an unexpected type in CreateCertificateFromCsr response." ) );
        }
        else
        {
            xCborRet = cbor_value_copy_text_string( &xValue, pcOwnershipTokenBuffer, pxOwnershipTokenBufferLength, NULL );

            if( xCborRet == CborErrorOutOfMemory )
            {
                size_t requiredLen = 0;
                ( void ) cbor_value_calculate_string_length( &xValue, &requiredLen );
                LogError( ( "Certificate ownership token buffer insufficiently large. Certificate ownership token buffer length: %lu", ( unsigned long ) requiredLen ) );
            }
            else if( xCborRet != CborNoError )
            {
                LogError( ( "Failed to parse \"certificateOwnershipToken\" value from CreateCertificateFromCsr response: %s.", cbor_error_string( xCborRet ) ) );
            }
        }
    }

    return( xCborRet == CborNoError );
}
/*-----------------------------------------------------------*/

CborError CloudProv_DeserializeThingName(const uint8_t * pucResponse,
                                    size_t xLength,
                                    char * pcThingNameBuffer,
                                    size_t * pxThingNameBufferLength )
{
    CborError cborRet;
    CborParser parser;
    CborValue map;
    CborValue value;

    configASSERT( pucResponse != NULL );
    configASSERT( pcThingNameBuffer != NULL );
    configASSERT( pxThingNameBufferLength != NULL );

    /* For details on the RegisterThing response payload format, see:
     * https://docs.aws.amazon.com/iot/latest/developerguide/fleet-provision-api.html#register-thing-response-payload
     */
    cborRet = cbor_parser_init( pucResponse, xLength, 0, &parser, &map );

    if( cborRet != CborNoError )
    {
        LogError( ( "Error initializing parser for RegisterThing response: %s.", cbor_error_string( cborRet ) ) );
    }
    else if( !cbor_value_is_map( &map ) )
    {
        LogError( ( "RegisterThing response not a map type." ) );
    }
    else
    {
        cborRet = cbor_value_map_find_value( &map, "thingName", &value );

        if( cborRet != CborNoError )
        {
            LogError( ( "Error searching RegisterThing response: %s.", cbor_error_string( cborRet ) ) );
        }
        else if( value.type == CborInvalidType )
        {
            LogError( ( "\"thingName\" not found in RegisterThing response." ) );
        }
        else if( value.type != CborTextStringType )
        {
            LogError( ( "\"thingName\" is an unexpected type in RegisterThing response." ) );
        }
        else
        {
            cborRet = cbor_value_copy_text_string( &value, pcThingNameBuffer, pxThingNameBufferLength, NULL );

            if( cborRet == CborErrorOutOfMemory )
            {
                size_t requiredLen = 0;
                ( void ) cbor_value_calculate_string_length( &value, &requiredLen );
                LogError( ( "Thing name buffer insufficiently large. Thing name length: %lu", ( unsigned long ) requiredLen ) );
            }
            else if( cborRet != CborNoError )
            {
                LogError( ( "Failed to parse \"thingName\" value from RegisterThing response: %s.", cbor_error_string( cborRet ) ) );
            }
        }
    }

    return cborRet;
}