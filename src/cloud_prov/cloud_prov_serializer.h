//
// Created by Gabriel on 3/23/2024.
//

#ifndef CLOUD_PROV_SERIALIZER_H
#define CLOUD_PROV_SERIALIZER_H

/*************************************************************************************
 *                                  INCLUDES
 ************************************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <cbor.h>

/*************************************************************************************
 *                             GLOBAL FUNCTION PROTOTYPES
 ************************************************************************************/
bool CloudProv_DeserializeCsrResponse(const uint8_t * pucResponse,
                                      size_t xLength,
                                      char * pcCertificateBuffer,
                                      size_t * pxCertificateBufferLength,
                                      char * pcCertificateIdBuffer,
                                      size_t * pxCertificateIdBufferLength,
                                      char * pcOwnershipTokenBuffer,
                                      size_t * pxOwnershipTokenBufferLength );

/**
 * @brief Creates the request payload to be published to the
 * CreateCertificateFromCsr API in order to request a certificate from AWS IoT
 * for the included Certificate Signing Request (CSR).
 * @param[in] csrBuffer The CSR to include in the request payload.
 * @param[in] csrLength The length of #csrBuffer.
 * @param[in] payloadBufferSize Length of #payloadBuffer.
 * @param[out] payloadBuffer Buffer into which to write the publish request payload.
 * @param[out] payloadLength The length of the publish request payload.
 */
bool CloudProv_SerializeCsr(const uint8_t * csrBuffer,
                            size_t csrLength,
                            size_t payloadBufferSize,
                            uint8_t * payloadBuffer,
                            size_t * payloadLength );

/**
 * @brief Creates the request payload to be published to the RegisterThing API
 * in order to activate the provisioned certificate and receive a Thing name.
 *
 * @param[in] certOwnershipToken The certificate's certificate ownership token.
 * @param[in] certOwnershipTokenLenght Length of #certificateOwnershipToken.
 * @param[in] serialNb Device serial number
 * @param[in] serialNbLenght lenght of serial number string
 * @param[in] payloadBufferSize Length of #buffer.
 * @param[out] payloadBuffer Buffer into which to write the publish request payload.
 * @param[out] payloadLength The length of the publish request payload.
 */
CborError CloudProv_SerializeRegisterThingRequest(const char *certOwnershipToken,
                                             size_t certOwnershipTokenLenght,
                                             const char *serialNb,
                                             size_t serialNbLenght,
                                             size_t payloadBufferSize,
                                             uint8_t *payloadBuffer,
                                             size_t *payloadLength );

CborError CloudProv_DeserializeThingName(const uint8_t * pucResponse,
                                         size_t xLength,
                                         char * pcThingNameBuffer,
                                         size_t * pxThingNameBufferLength );

#endif /* CLOUD_PROV_SERIALIZER_H */
