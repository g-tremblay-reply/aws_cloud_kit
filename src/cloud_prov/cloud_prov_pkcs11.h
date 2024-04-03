//
// Created by Gabriel on 3/23/2024.
//

#ifndef CLOUD_PROV_PKCS11_H
#define CLOUD_PROV_PKCS11_H

#include "mbedtls_pkcs11.h"

CK_RV CloudProv_GenerateKeyPairEC(CK_SESSION_HANDLE xSession,
                                  CK_OBJECT_HANDLE_PTR xPrivateKeyHandlePtr,
                                  CK_OBJECT_HANDLE_PTR xPublicKeyHandlePtr );

bool CloudProv_GenerateCsr(CK_SESSION_HANDLE xP11Session,
                           uint8_t * pcCsrBuffer,
                           size_t xCsrBufferLength,
                           size_t * pxOutCsrLength );

bool CloudProv_LoadCertificate(CK_SESSION_HANDLE xP11Session,
                               const char * pcCertificate,
                               size_t xCertificateLength );

#endif /*CLOUD_PROV_PKCS11_H*/
