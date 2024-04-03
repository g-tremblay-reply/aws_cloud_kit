//
// Created by Gabriel on 3/23/2024.
//

/*************************************************************************************
 *                                  INCLUDES
 ************************************************************************************/
#include <cloud_prov_pkcs11.h>
#include <cloud_prov_config.h>
#include <console.h>
#include <x509_csr.h>
#include <mbedtls_utils.h>


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

CK_RV CloudProv_GenerateKeyPairEC(CK_SESSION_HANDLE xSession,
                                  CK_OBJECT_HANDLE_PTR xPrivateKeyHandlePtr,
                                  CK_OBJECT_HANDLE_PTR xPublicKeyHandlePtr )
{
    CK_RV xResult;
    CK_MECHANISM xMechanism = { CKM_EC_KEY_PAIR_GEN, NULL_PTR, 0 };
    CK_FUNCTION_LIST_PTR xFunctionList;
    CK_BYTE pxEcParams[] = pkcs11DER_ENCODED_OID_P256; /* prime256v1 */
    CK_KEY_TYPE xKeyType = CKK_EC;

    CK_BBOOL xTrueObject = CK_TRUE;
    CK_ATTRIBUTE pxPublicKeyTemplate[] =
            {
                    { CKA_KEY_TYPE,  NULL /* &keyType */,         sizeof( xKeyType )             },
                    { CKA_VERIFY,    NULL /* &trueObject */,      sizeof( xTrueObject )          },
                    { CKA_EC_PARAMS, NULL /* ecParams */,         sizeof( pxEcParams )           },
                    { CKA_LABEL,     ( void * ) pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS, strnlen( pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS, pkcs11configMAX_LABEL_LENGTH )}
            };

    /* Aggregate initializers must not use the address of an automatic variable. */
    pxPublicKeyTemplate[ 0 ].pValue = &xKeyType;
    pxPublicKeyTemplate[ 1 ].pValue = &xTrueObject;
    pxPublicKeyTemplate[ 2 ].pValue = &pxEcParams;

    CK_ATTRIBUTE privateKeyTemplate[] =
            {
                    { CKA_KEY_TYPE, NULL /* &keyType */,          sizeof( xKeyType )             },
                    { CKA_TOKEN,    NULL /* &trueObject */,       sizeof( xTrueObject )          },
                    { CKA_PRIVATE,  NULL /* &trueObject */,       sizeof( xTrueObject )          },
                    { CKA_SIGN,     NULL /* &trueObject */,       sizeof( xTrueObject )          },
                    { CKA_LABEL,    ( void * ) pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS, strnlen( pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS, pkcs11configMAX_LABEL_LENGTH )}
            };

    /* Aggregate initializers must not use the address of an automatic variable. */
    privateKeyTemplate[ 0 ].pValue = &xKeyType;
    privateKeyTemplate[ 1 ].pValue = &xTrueObject;
    privateKeyTemplate[ 2 ].pValue = &xTrueObject;
    privateKeyTemplate[ 3 ].pValue = &xTrueObject;

    xResult = C_GetFunctionList( &xFunctionList );

    if( xResult != CKR_OK )
    {
        LogError( ( "Could not get a PKCS #11 function pointer." ) );
    }
    else
    {
        xResult = xFunctionList->C_GenerateKeyPair( xSession,
                                                    &xMechanism,
                                                    pxPublicKeyTemplate,
                                                    sizeof( pxPublicKeyTemplate ) / sizeof( CK_ATTRIBUTE ),
                                                    privateKeyTemplate,
                                                    sizeof( privateKeyTemplate ) / sizeof( CK_ATTRIBUTE ),
                                                    xPublicKeyHandlePtr,
                                                    xPrivateKeyHandlePtr );
    }

    return xResult;
}

bool CloudProv_GenerateCsr(CK_SESSION_HANDLE xP11Session,
                           uint8_t * pcCsrBuffer,
                           size_t xCsrBufferLength,
                           size_t * pxOutCsrLength ) {
    CK_OBJECT_HANDLE xPrivKeyHandle;
    CK_OBJECT_HANDLE xPubKeyHandle;
    CK_RV xPkcs11Ret = CKR_OK;
    mbedtls_pk_context xPrivKey;
    mbedtls_ecdsa_context xEcdsaContext;
    mbedtls_x509write_csr xReq;
    int32_t ulMbedtlsRet = -1;
    const mbedtls_pk_info_t *pxHeader = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);


    /* Current corePKCS11 integration does not allow for new certificate and private key, thus
         * destroy claim credentials to free their spot in corePKCS's memory integration (littleFS) */
    xPkcs11Ret = xDestroyDefaultCryptoObjects(xP11Session );

    if (xPkcs11Ret == CKR_OK)
    {
        xPkcs11Ret = CloudProv_GenerateKeyPairEC(xP11Session,
                                                 &xPrivKeyHandle,
                                                 &xPubKeyHandle);
    }

    if (xPkcs11Ret == CKR_OK)
    {
        xPkcs11Ret = xPKCS11_initMbedtlsPkContext(&xPrivKey, xP11Session, xPrivKeyHandle);
    }

    if (xPkcs11Ret == CKR_OK)
    {
        mbedtls_x509write_csr_init(&xReq);
        mbedtls_x509write_csr_set_md_alg(&xReq, MBEDTLS_MD_SHA256);

        ulMbedtlsRet = mbedtls_x509write_csr_set_key_usage(&xReq, MBEDTLS_X509_KU_DIGITAL_SIGNATURE);

        if (ulMbedtlsRet == 0) {
            ulMbedtlsRet = mbedtls_x509write_csr_set_ns_cert_type(&xReq, MBEDTLS_X509_NS_CERT_TYPE_SSL_CLIENT);
        }

        if (ulMbedtlsRet == 0) {
            ulMbedtlsRet = mbedtls_x509write_csr_set_subject_name(&xReq, democonfigCSR_SUBJECT_NAME);
        }

        if (ulMbedtlsRet == 0) {
            mbedtls_x509write_csr_set_key(&xReq, &xPrivKey);

            ulMbedtlsRet = mbedtls_x509write_csr_pem(&xReq,
                                                     (unsigned char *) pcCsrBuffer,
                                                     xCsrBufferLength,
                                                     &lMbedCryptoRngCallbackPKCS11,
                                                     &xP11Session);
        }

        if( ulMbedtlsRet != 0U )
        {
            APP_ERR_PRINT( ( "Failed to generate Key and Certificate Signing Request.\r\n" ) );
        }

        mbedtls_x509write_csr_free(&xReq);

        mbedtls_pk_free(&xPrivKey);
    }

    *pxOutCsrLength = strlen(pcCsrBuffer);

    return (ulMbedtlsRet == 0);
}


bool CloudProv_LoadCertificate(CK_SESSION_HANDLE xP11Session,
                               const char * pcCertificate,
                               size_t xCertificateLength )
{
    PKCS11_CertificateTemplate_t xCertificateTemplate;
    CK_OBJECT_CLASS xCertificateClass = CKO_CERTIFICATE;
    CK_CERTIFICATE_TYPE xCertificateType = CKC_X_509;
    CK_FUNCTION_LIST_PTR xFunctionList = NULL;
    CK_RV xResult = CKR_OK;
    uint8_t * pucDerObject = NULL;
    int32_t ulConversion = 0;
    size_t xDerLen = 0;
    CK_BBOOL xTokenStorage = CK_TRUE;
    CK_BYTE pxSubject[] = "TestSubject";
    CK_OBJECT_HANDLE xObjectHandle = CK_INVALID_HANDLE;

    if( pcCertificate == NULL )
    {
        LogError( ( "Certificate cannot be null." ) );
        xResult = CKR_ATTRIBUTE_VALUE_INVALID;
    }

    if( xResult == CKR_OK )
    {
        /* Convert the certificate to DER format from PEM. The DER key should
         * be about 3/4 the size of the PEM key, so mallocing the PEM key size
         * is sufficient. */
        pucDerObject = ( uint8_t * ) malloc( xCertificateLength + 1 );
        xDerLen = xCertificateLength + 1;

        if( pucDerObject != NULL )
        {
            ulConversion = convert_pem_to_der( ( unsigned char * ) pcCertificate,
                                               xCertificateLength + 1,
                                               pucDerObject, &xDerLen );

            if( 0 != ulConversion )
            {
                LogError( ( "Failed to convert provided certificate." ) );
                xResult = CKR_ARGUMENTS_BAD;
            }
        }
        else
        {
            LogError( ( "Failed to allocate buffer for converting certificate to DER." ) );
            xResult = CKR_HOST_MEMORY;
        }
    }

    if( xResult == CKR_OK )
    {
        xResult = C_GetFunctionList( &xFunctionList );

        if( xResult != CKR_OK )
        {
            LogError( ( "Could not get a PKCS #11 function pointer." ) );
        }
    }

    if( xResult == CKR_OK )
    {
        /* Initialize the client certificate template. */
        xCertificateTemplate.xObjectClass.type = CKA_CLASS;
        xCertificateTemplate.xObjectClass.pValue = &xCertificateClass;
        xCertificateTemplate.xObjectClass.ulValueLen = sizeof( xCertificateClass );
        xCertificateTemplate.xSubject.type = CKA_SUBJECT;
        xCertificateTemplate.xSubject.pValue = pxSubject;
        xCertificateTemplate.xSubject.ulValueLen = strlen( ( const char * ) pxSubject );
        xCertificateTemplate.xValue.type = CKA_VALUE;
        xCertificateTemplate.xValue.pValue = pucDerObject;
        xCertificateTemplate.xValue.ulValueLen = xDerLen;
        xCertificateTemplate.xLabel.type = CKA_LABEL;
        xCertificateTemplate.xLabel.pValue = ( CK_VOID_PTR ) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
        xCertificateTemplate.xLabel.ulValueLen = strnlen( pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS, pkcs11configMAX_LABEL_LENGTH );
        xCertificateTemplate.xCertificateType.type = CKA_CERTIFICATE_TYPE;
        xCertificateTemplate.xCertificateType.pValue = &xCertificateType;
        xCertificateTemplate.xCertificateType.ulValueLen = sizeof( CK_CERTIFICATE_TYPE );
        xCertificateTemplate.xTokenObject.type = CKA_TOKEN;
        xCertificateTemplate.xTokenObject.pValue = &xTokenStorage;
        xCertificateTemplate.xTokenObject.ulValueLen = sizeof( xTokenStorage );

        /* Create an object using the encoded client certificate. */
        LogInfo( ( "Writing certificate into label \"%s\".", pcLabel ) );

        xResult = xFunctionList->C_CreateObject( xP11Session,
                                                 ( CK_ATTRIBUTE_PTR ) &xCertificateTemplate,
                                                 sizeof( xCertificateTemplate ) / sizeof( CK_ATTRIBUTE ),
                                                 &xObjectHandle );
    }

    if( pucDerObject != NULL )
    {
        free( pucDerObject );
    }

    return( xResult == CKR_OK );
}

