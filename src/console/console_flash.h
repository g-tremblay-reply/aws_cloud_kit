/***********************************************************************************************************************
 * File Name    : console_flash.h
 * Description  : Contains macros, data structures and functions used in flash_hp.h
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#include <console.h>
#include <cloud_prov.h>

#ifndef CONSOLE_FLASH_H
#define CONSOLE_FLASH_H


/* Declare stored status string and its length to write into the data flash when the credential stored */
#define CONSOLE_FLASH_SAVE					   "SAVEDONE"
#define CONSOLE_FLASH_LENGTH_SAVE					   (8)

typedef struct s_credentials_mem_map
{
	uint32_t num_bytes;
	uint32_t addr;
	uint8_t num_block;
	char stored_in_flash[CONSOLE_FLASH_LENGTH_SAVE];
	uint32_t length;
}ConsoleCredentialMem_t;

typedef enum
{
    CONSOLE_CERTIFICATE =           0u,
    CONSOLE_RSA_PRIVATE_KEY =           1u,
    CONSOLE_MQTT_ENDPOINT =         2u,
    CONSOLE_IOT_THING_NAME =        3u,
    CONSOLE_FLASH_DATA_INFO =       4u,
}ConsoleCredential_t;

#define CONSOLE_FLASH_CREDENTIALS_COUNT (5)
#define FLASH_HP_DF_BLOCK_SIZE            (64)

/* Data Flash */
#define FLASH_HP_DF_BLOCK_0               (0x08001000U) /*   64 B:    0x80001000 - 0x8000103F */
#define FLASH_HP_DF_BLOCK_CERTIFICATE     (0x08001040U) /*   1536 B:  0x08001040 - 0x0800163F */
#define FLASH_HP_DF_BLOCK_KEY             (0x08001640U) /*   2048 B:  0x08001640 - 0x08001E3F */
#define FLASH_HP_DF_MQTT_END_POINT        (0x08001E40U) /*   128 B:   0x08001E40 - 0x08001EBF */
#define FLASH_HP_DF_IOT_THING_NAME        (0x08001EC0U) /*   128 B:   0x08001EC0 - 0x08001F3F */
#define FLASH_HP_DF_DATA_INFO             (0x08001F40U) /*   128 B:   0x08001F40 - 0x08001FBF */


#define TOTAL_BLOCK_SIZE                  (3968)
#define TOTAL_BLOCK_NUM					  (62)

#define BLOCK_SIZE_CERT                   (1536)
#define BLOCK_NUM_CERT			          (24)

#define BLOCK_SIZE_KEY                    (2048)
#define BLOCK_NUM_KEY			          (32)

#define BLOCK_SIZE_MQTT_ENDPOINT          (128)
#define BLOCK_NUM_MQTT_ENDPOINT	          (2)

#define BLOCK_SIZE_IOT_THING              (128)
#define BLOCK_NUM_IOT_THING		          (2)

#define BLOCK_SIZE_DATA_INFO              (128)
#define BLOCK_NUM_DATA_INFO		          (2)
//
#define BUFFER_SIZE                       (2048)

extern bool ConsoleMqttEndpointStored;
extern bool ConsoleClaimCertificateStored;
extern bool ConsoleClaimPrivateKeyStored;


/*flash_hp operating functions */
fsp_err_t Console_StoreAwsCredential(ConsoleCredential_t credentialType, const char *credentialBuffer, uint32_t credentialLength);
void Console_FlashDeinit(void);
void Console_FlashSizesInfo(uint32_t *physicalSize, uint32_t *allocatedSize, uint32_t *freeSize);
void Console_ResetClaimCredentials(void);
void Console_LoadCredentialsFromFlash (void);
void Console_CheckStoredCredentials (void);

#endif /* CONSOLE_FLASH_H */
