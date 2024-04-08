/***********************************************************************************************************************
 * File Name    : console_flash.c
 * Description  : Contains macros, data structures and functions used in flash_hp.c
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
#include <console_flash.h>
#include <cloud_prov.h>

static fsp_err_t Console_WaitBlankCheck(void);
static fsp_err_t Console_WriteToFlash(ConsoleCredentialMem_t *memMapItem, uint8_t *credentialBuffer);


static ConsoleCredentialMem_t ConsoleDataFlashInfo[5];
static bool is_b_flash_event_not_blank = false;
static bool is_b_flash_event_blank = false;
static bool is_b_flash_event_erase_complete = false;
static bool is_b_flash_event_write_complete = false;

/*******************************************************************************************************************//**
 * @brief This function is called to set the blank check event flags
 * @param[IN]   None
 * @retval      FSP_SUCCESS             Upon successful Flash_HP is blank
 * @retval      Any Other Error code    Upon unsuccessful Flash_HP is not blank
 **********************************************************************************************************************/
static fsp_err_t Console_WaitBlankCheck(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Wait for callback function to set flag */
    while (!(is_b_flash_event_not_blank || is_b_flash_event_blank));
    if (is_b_flash_event_not_blank)
    {
        APP_ERR_PRINT("\n\rFlash is not blank, not to write the data. Restart the application\n");
        /* Reset Flag */
        is_b_flash_event_not_blank = false;
        return (fsp_err_t) FLASH_EVENT_NOT_BLANK;
    }
    else
    {
        APP_PRINT("\r\nFlash is blank\n");
        /* Reset Flag */
        is_b_flash_event_blank = false;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief This function is called from the RTT input to do the data flash operations.
 * @param[IN]   None
 * @retval      FSP_SUCCESS             Upon successful FLash_HP data flash operations.
 * @retval      Any Other Error code    Upon unsuccessful Flash_HP data flash operations.
 **********************************************************************************************************************/
static fsp_err_t Console_WriteToFlash(ConsoleCredentialMem_t *memMapItem, uint8_t *credentialBuffer)
{
    fsp_err_t err;
    flash_result_t blank_check_result = FLASH_RESULT_BLANK;

    /* Erase Block */
    err = R_FLASH_HP_Erase (&user_flash_ctrl, memMapItem->addr, memMapItem->num_block);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_DBG_PRINT("\r\nErase API failed, Restart the Application\r\n");
        return err;
    }

    /* Wait for the erase complete event flag, if BGO is SET  */
    if (true == user_flash_cfg.data_flash_bgo)
    {
        APP_DBG_PRINT("\r\nBGO has enabled, Data Flash erase is in progress\r\n");
        while (!is_b_flash_event_erase_complete);
        is_b_flash_event_erase_complete = false;
    }

    /* Data flash blank check */
    err = R_FLASH_HP_BlankCheck (&user_flash_ctrl, memMapItem->addr,
                                 FLASH_HP_DF_BLOCK_SIZE, &blank_check_result);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nBlankCheck API failed, Restart the Application\r\n");
        return err;
    }

    /* Validate the blank check result */
    if (FLASH_RESULT_BLANK == blank_check_result)
    {
        APP_DBG_PRINT("\r\n BlankCheck is successful\r\n");
    }
    else if (FLASH_RESULT_NOT_BLANK == blank_check_result)
    {
        APP_ERR_PRINT("\r\n BlankCheck failed. cannot write the data. Try Restarting the application\r\n");
    }
    else if (FLASH_RESULT_BGO_ACTIVE == blank_check_result)
    {
        /* BlankCheck will update in Callback */
        /* Event flag will be updated in the blank check function when BGO is enabled */
        err = Console_WaitBlankCheck();
    }
    else
    {
        /* No Operation */
    }

    /* Stores details of credentials into flash memory*/
    err = R_FLASH_HP_Write (&user_flash_ctrl, (uint32_t)credentialBuffer,
                            memMapItem->addr, memMapItem->num_bytes);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_FLASH_HP_Write API failed, Restart the Application\r\n");
    }

    /* Wait for the write complete event flag, if BGO is SET  */
    if (true == user_flash_cfg.data_flash_bgo)
    {
        while (!is_b_flash_event_write_complete);
        is_b_flash_event_write_complete = false;
    }

    return err;
}


bool Console_LoadCredentialsFromFlash(void)
{
    bool mqttEndpointStored = false;
    /* Load Data flash info from flash to RAM */
    memcpy (ConsoleDataFlashInfo, (ConsoleCredentialMem_t*) FLASH_HP_DF_DATA_INFO, sizeof(ConsoleDataFlashInfo));

    /* Init SAVED status to 0 if not credential not saved */
    for(uint8_t credential=0u; credential < CONSOLE_FLASH_CREDENTIALS_COUNT; credential++)
    {
        if (0 != strncmp ((char *)ConsoleDataFlashInfo[credential].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
        {
            memset (ConsoleDataFlashInfo[credential].stored_in_flash, 0, CONSOLE_FLASH_LENGTH_SAVE);
        }
    }

    /* Check if mqtt endpoint already saved */
    if(0 == strncmp ((char *)ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
    {
        mqttEndpointStored = true;
        /* Import endpoint to CloudProv module, allowing it to use this stored endpoint
         * stored in flash instead of default endpoint when connecting to MQTT broker */
        CloudProv_ImportMqttEndpoint((uint8_t *)ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].addr,
                                     ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].length);
        APP_WARN_PRINT("\r\n\r\nMQTT Broker Endpoint is present in flash memory. Using it instead of default one.[WHITE]\r\n\r\n");

    }

    /* Init hardcoded addresses, sizes and block num values */
    ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].num_bytes = BLOCK_SIZE_CERT;
    ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].num_block = BLOCK_NUM_CERT;
    ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].addr = FLASH_HP_DF_BLOCK_CERTIFICATE;

    ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].num_bytes = BLOCK_SIZE_KEY;
    ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].num_block = BLOCK_NUM_KEY;
    ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].addr = FLASH_HP_DF_BLOCK_KEY;

    ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].num_bytes = BLOCK_SIZE_MQTT_ENDPOINT;
    ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].num_block = BLOCK_NUM_MQTT_ENDPOINT;
    ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].addr = FLASH_HP_DF_MQTT_END_POINT;

    ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].num_bytes = BLOCK_SIZE_IOT_THING;
    ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].num_block = BLOCK_NUM_IOT_THING;
    ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].addr = FLASH_HP_DF_IOT_THING_NAME;

    ConsoleDataFlashInfo[CONSOLE_FLASH_DATA_INFO].num_bytes = BLOCK_SIZE_DATA_INFO;
    ConsoleDataFlashInfo[CONSOLE_FLASH_DATA_INFO].num_block = BLOCK_NUM_DATA_INFO;
    ConsoleDataFlashInfo[CONSOLE_FLASH_DATA_INFO].addr = FLASH_HP_DF_DATA_INFO;

    return mqttEndpointStored;
}



/*******************************************************************************************************************//**
 * @brief This functions provide flash memory information
 **********************************************************************************************************************/
void Console_FlashSizesInfo(uint32_t *physicalSize, uint32_t *allocatedSize, uint32_t *freeSize)
{
    uint32_t totalMem, allocatedMem, freeMem;

    /* Total available flash memory */
    totalMem = TOTAL_BLOCK_SIZE;

    for(uint8_t memSlot=0u; memSlot<CONSOLE_FLASH_CREDENTIALS_COUNT; memSlot++)
    {
        if (0 == strncmp ((char *)ConsoleDataFlashInfo[memSlot].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
        {
            allocatedMem += ConsoleDataFlashInfo[memSlot].length;
        }
    }

    /* Free size available in flash memory */
    freeMem = totalMem - allocatedMem;
    /* copy memory sizes to output parameters */
    *physicalSize = totalMem;
    *allocatedSize = allocatedMem;
    *freeSize = freeMem;
}

void Consol_CheckStoredCredentials(void)
{
    char read_buffer[2048]= {0};

    /* Check if credential is stored in flash */
    if (0 == strncmp ((char *)ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
    {
        /* Copy credential from flash memory to buffer */
        memcpy (read_buffer, (uint8_t*) ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].addr, ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].num_bytes);
        /* Validate credential */
        if ((NULL != strstr (read_buffer, "-----END CERTIFICATE-----"))
                && (strlen (read_buffer) == ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].length))
        {
            Console_ColorPrintf("\r\n [GREEN]Claim Certificate saved in data flash is correctly formatted[WHITE]\r\n");
            memset (read_buffer, 0, strlen (read_buffer));
        }
        else
        {
            Console_ColorPrintf("\r\n [RED]Claim Certificate saved in data flash is invalid[WHITE]\r\n");
            memset (ConsoleDataFlashInfo[CONSOLE_CERTIFICATE].stored_in_flash, 0, CONSOLE_FLASH_LENGTH_SAVE);
        }
    }
    else
    {
        Console_ColorPrintf("\r\n [RED]No Claim Certificate is saved in data flash\r\n"
                            "Cloud Application will try to use default value[WHITE]\r\n");
    }

    /* Check if credential is stored in flash */
    if (0 == strncmp ((char *)ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
    {
        /* Copy credential from flash memory to buffer */
        memcpy (read_buffer, (uint8_t*) ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].addr, ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].num_bytes);
        /* Validate credential */
        if ((NULL != strstr (read_buffer, "-----END RSA PRIVATE KEY-----"))
                && (strlen (read_buffer) == ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].length))
        {
            Console_ColorPrintf("\r\n [GREEN]Claim Private key saved in data flash is correctly formatted[WHITE]\r\n");
            memset (read_buffer, 0, strlen (read_buffer));
        }
        else
        {
            Console_ColorPrintf("\r\n [RED]Claim Private key saved in data flash is invalid[WHITE]\r\n");
            memset (ConsoleDataFlashInfo[CONSOLE_PRIVATE_KEY].stored_in_flash, 0, CONSOLE_FLASH_LENGTH_SAVE);
        }
    }
    else
    {
        Console_ColorPrintf("\r\n [RED]No Claim Private key is saved in data flash\r\n"
                            "Cloud Application will try to use default value[WHITE]\r\n");
    }

    /* Check if credential is stored in flash */
    if (0 == strncmp ((char *)ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
    {
        /* Copy credential from flash memory to buffer */
        memcpy (read_buffer, (uint8_t*) ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].addr,
                ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].num_bytes);
        /* Validate credential. Increment length +1 here because this credential is inputted by pressing
         * ENTER in user input, which adds a \r char. Thus, when saving .length parameter,  \r was character
         * was removed from count for easier string manipulations */
        if (strlen (read_buffer) == (ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].length +1u))
        {
            Console_ColorPrintf("\r\n [GREEN]MQTT end point saved in data flash has valid length[WHITE]\r\n");
            Console_ColorPrintf("\r\n     >[ORANGE]%s[WHITE]\r\n", read_buffer);
            memset (read_buffer, 0, strlen (read_buffer));
        }
        else
        {
            Console_ColorPrintf("\r\n [RED]MQTT endpoint from flash memory has invalid lenth.[WHITE]\r\n");
            memset (ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].stored_in_flash, 0, CONSOLE_FLASH_LENGTH_SAVE);
        }
    }
    else
    {
        Console_ColorPrintf("\r\n [RED]No MQTT end point is not saved in data flash.\r\n"
                            "Cloud Application will try to use default value[WHITE]\r\n");
    }

    /* Check if credential is stored in flash */
    if (0 == strncmp ((char *)ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].stored_in_flash, (char *)CONSOLE_FLASH_SAVE, CONSOLE_FLASH_LENGTH_SAVE))
    {
        /* Copy credential from flash memory to buffer */
        memcpy (read_buffer, (uint8_t*) ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].addr,
                ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].num_bytes);
        /* Validate credential. Increment length +1 here because this credential is inputted by pressing
        * ENTER in user input, which adds a \r char. Thus, when saving .length parameter,  \r was character
        * was removed from count for easier string manipulations */
        if (strlen (read_buffer) == ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].length +1u)
        {
            Console_ColorPrintf("\r\n [GREEN]IOT thing name saved in data flash has valid length[WHITE]\r\n\r\n");
            Console_ColorPrintf("\r\n     >[ORANGE]%s[WHITE]\r\n", read_buffer);

            memset (read_buffer, 0, strlen (read_buffer));
        }
        else
        {
            Console_ColorPrintf(
                    "\r\n [RED]IOT thing name saved in data flash has invalid length[WHITE]\r\n");
            memset (ConsoleDataFlashInfo[CONSOLE_IOT_THING_NAME].stored_in_flash, 0, CONSOLE_FLASH_LENGTH_SAVE);
        }
    }
    else
    {
        Console_ColorPrintf("\r\n [RED]No IOT thing name is saved in data flash.\r\n"
                            "Cloud Application will try to use default value[WHITE]\r\n");
    }
}

void Console_FlashDeinit(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_FLASH_HP_Close (&user_flash_ctrl);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nClose API failed in BlankCheck API");
    }
}

/*******************************************************************************************************************//**
 * @brief Callback function for FLASH HP HAL
 * @param[IN]   p_args                  Pointer to callback argumet
 * @retval
 * @retval
 **********************************************************************************************************************/
void Console_FlashCallback(flash_callback_args_t *p_args)
{
    if (FLASH_EVENT_NOT_BLANK == p_args->event)
    {
        is_b_flash_event_not_blank = true;
    }
    else if (FLASH_EVENT_BLANK == p_args->event)
    {
        is_b_flash_event_blank = true;
    }
    else if (FLASH_EVENT_ERASE_COMPLETE == p_args->event)
    {
        is_b_flash_event_erase_complete = true;
    }
    else if (FLASH_EVENT_WRITE_COMPLETE == p_args->event)
    {
        is_b_flash_event_write_complete = true;
    }
    else
    {
        /*No operation */
    }
}


fsp_err_t Console_StoreAwsCredential(ConsoleCredential_t credentialType, const char *credentialBuffer, uint32_t credentialLength)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Store credential in flash memory */
    ConsoleDataFlashInfo[credentialType].length = credentialLength;
    err = Console_WriteToFlash(&ConsoleDataFlashInfo[credentialType],(uint8_t *)credentialBuffer);

    if (err == FSP_SUCCESS)
    {
        Console_ColorPrintf("\r\n[GREEN]Credential successfully writen to flash.\r\n"
                                   "    >[ORANGE]%s[WHITE]\r\n", credentialBuffer);
        /* Store flash data info in flash memory. This basically serves the purpose of knowing at application
         * startup if data was saved previously in flash or not, with standard string labels */
        strcpy((char *)ConsoleDataFlashInfo[credentialType].stored_in_flash, (char *)CONSOLE_FLASH_SAVE);

        err = Console_WriteToFlash(&ConsoleDataFlashInfo[CONSOLE_FLASH_DATA_INFO],
                                   (uint8_t *)ConsoleDataFlashInfo);

        if (err == FSP_SUCCESS)
        {
            Console_ColorPrintf("\r\n[GREEN]Data flash info successfully writen to flash.[WHITE]\r\n");
        }
        else
        {
            Console_ColorPrintf("\r\n[RED]Data flash info write to flash failed.[WHITE]\r\n");
        }

        if(credentialType == CONSOLE_MQTT_ENDPOINT)
        {
            CloudProv_ImportMqttEndpoint((uint8_t *)ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].addr,
                                         ConsoleDataFlashInfo[CONSOLE_MQTT_ENDPOINT].length);
        }
    }
    else
    {
        Console_ColorPrintf("\r\n[RED]Credential write to flash failed.[WHITE]\r\n");
    }

    return err;
}
