/***********************************************************************************************************************
 * File Name    : console.c
 * Description  : Contains data structures and functions used for console
 **********************************************************************************************************************/
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
#include "console.h"
#include "console_flash.h"


#define AP_VERSION      ("2.0")
#define MODULE_NAME     "AWS Core MQTT"
#define BANNER_INFO     "\r\n[CYAN]********************************************************************************"\
                        "\r\n*   Renesas FSP Application Project for "MODULE_NAME"                          *"\
                        "\r\n*   Application Project Version %s                                            *"\
                        "\r\n*   Flex Software Pack Version  %d.%d.%d                                          *"\
                        "\r\n*   Device's MCU 128-bit Unique ID (hex) : %08x-%08x-%08x-%08x *"\
                        "\r\n********************************************************************************"\
                        "\r\nRefer to Application Note for more details on Application Project and              " \
                        "\r\nFSP User's Manual for more information about "MODULE_NAME"                    "\
                        "\r\n********************************************************************************[WHITE]\r\n"
#define CONSOLE_MENU_RETURN         "\r\n\r\n> Press BACKSPACE key to return to MENU\r\n"
#define CONSOLE_SUB_OPTIONS         "\r\n> Select from the options in the menu below:\r\n"
#define CONSOLE_FLASH_CHECK_CREDENTIALS        "\r\nCHECK CREDENTIALS STORED IN DATA FLASH\r\n"
#define CONSOLE_END_OF_RSA_PRIVATE_KEY_SUBSTR         "-----END RSA PRIVATE KEY-----"
#define CONSOLE_END_OF_CERTIFICATE_SUBSTR         "-----END CERTIFICATE-----"
#define CONSOLE_CONNECTION_ABORT    (0x00)
#define CONSOLE_MENU_BACKSPACE_KEY  (0x08)
#define CONSOLE_MENU_EXIT_KEY       CONSOLE_MENU_BACKSPACE_KEY
#define CONSOLE_ENTER_KEY           '\r'
#define CONSOLE_CLEAR_SCREEN        "\x1b[2J"
#define CONSOLE_CURSOR_HOME         "\x1b[H"
#define CONSOLE_CURSOR_STORE        "\x1b[s"
#define CONSOLE_CURSOR_RESTORE      "\x1b[u"
#define CONSOLE_CURSOR_TEMP         "\x1b[8;41H\x1b[K"
#define CONSOLE_CURSOR_FREQUENCY    "\x1b[9;41H\x1b[K"
#define CONSOLE_CURSOR_INTENSITY    "\x1b[10;41H\x1b[K"

typedef struct
{
    char * menuName;
    void ( * displayMenu)(int8_t);
} ConsoleMenu_t;


static char Console_WaitForKeypress(void);
static void Console_HelpMenu (int8_t selectedMenu);
static void Console_DefaultClaimCredMenu (int8_t selectedMenu);
static void Console_StartAppMenu(int8_t selectedMenu);
static void Console_FlashInfoMenu(int8_t selectedMenu);
static void Console_FlashWriteCertMenu(int8_t selectedMenu);
static void Console_FlashWriteKeyMeu (int8_t selectedMenu);
static void Console_FlashWriteIotNameMenu(int8_t selectedMenu);
static void Console_FlashWriteEndPointMenu(int8_t selectedMenu);
static void Console_FlashCheckCredentialsMenu(int8_t selectedMenu);
static char Console_ParseUserCredentials(ConsoleCredential_t credential);


extern TaskHandle_t cloud_app_thread; // @suppress("Global (API or Non-API) variable prefix")

static uint8_t  ConsoleInputBuffer[TRANSFER_LENGTH] = {0};
static bool ConsoleTransmitDone = false;
static bool ConsoleUserInputReceived  = false;
static uint32_t ConsoleInputIndex = 0;
static char ConsoleCredentialBuffer[TRANSFER_LENGTH]= {0};
static uint8_t ConsoleReadCredential = false;
static uint16_t ConsoleCredentialIndex = 0;
static fsp_pack_version_t ConsoleFSPversion = {0u};
static bsp_unique_id_t const *ConsoleDeviceUUID;

/* Table of main menu functions */
static ConsoleMenu_t ConsoleMainMenus[] =
        {
                {"Start Application"             , Console_StartAppMenu},
                {"Write MQTT Broker end point"   , Console_FlashWriteEndPointMenu},
                {"Write Claim Certificate"             , Console_FlashWriteCertMenu},
                {"Write RSA Claim Private Key"             , Console_FlashWriteKeyMeu},
                {"Write IOT Thing name"          , Console_FlashWriteIotNameMenu},
                {"Check stored credentials"      ,Console_FlashCheckCredentialsMenu },
                {"Data Flash Info"               , Console_FlashInfoMenu},
                {"Use default Claim Credentials" , Console_DefaultClaimCredMenu},
                {"Help"                          , Console_HelpMenu},
                {"", NULL }
        };

/*********************************************************************************************************************
 * Function Name: Console_GetColour
 *                Get the escape code string for the supplied colour tag
 * @param[in] char *string : the escape code
 * @param[out] bool *found : true if the tag was found, false if not
 * @retval the escape code for the colour tag, or the original string if there
 *         was no match
*********************************************************************************************************************/
static const char *Console_GetColour(const char *string, bool *found)
{
    const char *colour_codes[] = {"[BLACK]", "\x1B[30m", "[RED]", "\x1B[91m", "[GREEN]", "\x1B[92m", "[YELLOW]", "\x1B[93m",
                                  "[BLUE]", "\x1B[94m", "[MAGENTA]", "\x1B[95m", "[CYAN]", "\x1B[96m", "[WHITE]", "\x1B[97m",
                                  "[ORANGE]", "\x1B[38;5;208m", "[PINK]", "\x1B[38;5;212m", "[BROWN]", "\x1B[38;5;94m",
                                  "[PURPLE]", "\x1B[35m"};
	uint8_t index;

    for (index = 0; index < 12; index++)
    {
        if (0 == strcmp (string, colour_codes[index << 1]))
        {
            *found = true;
            return colour_codes[(index << 1) + 1];
        }
    }

    *found = false;
    return (string);
}

/*********************************************************************************************************************
 * Function Name: Console_Detokenise
 *                Replace colour tokens with terminal colour escape codes
 * @param[in] char *input : input string possibly containing colour tokens
 * @param[in] output *output : string with colour tokens replaced with escape codes
 * @retval
 *
*********************************************************************************************************************/
static void Console_Detokenise(const char * input, char *output)
{
    int16_t start_bracket_index;
    int16_t end_bracket_index;
    int16_t start_bracket_output_index;
    size_t token_length;
    int16_t index;
    int16_t o_index;
    bool token_found;
    bool token_replaced;
    const char *colour_code;
    char token[MAXIMUM_TOKEN_LENGTH + 1];

    start_bracket_index = -1;
    end_bracket_index = -1;
    start_bracket_output_index = 0;
    o_index = 0;

    /* scan the input string */
    for (index = 0; '\0' != input[index]; index++)
    {
        token_replaced = false;

        /* token start? */
        if ('[' == input[index])
        {
            start_bracket_index = index;
            start_bracket_output_index = o_index;
        }

        /* token end? */
        if (']' == input[index])
        {
            end_bracket_index = index;

            /* check to see if we have a token */
            if (start_bracket_index >= 0)
            {
                token_length = (size_t) (end_bracket_index - start_bracket_index + 1);

                if ((token_length >= MINIMUM_TOKEN_LENGTH) && (token_length <= MAXIMUM_TOKEN_LENGTH))
                {
                    /* copy the token to a buffer */
                    strncpy (token, &input[start_bracket_index], token_length);
                    token[token_length] = '\0';

                    /* check for a valid token */
                    colour_code = Console_GetColour(token, &token_found);

                    /* if we have a colour token, then replace it in the output with the associated escape code */
                    if (token_found)
                    {
                        strcpy (&output[start_bracket_output_index], colour_code);
                        o_index = (int16_t) (start_bracket_output_index + (int16_t) strlen (colour_code));
                        token_replaced = true;
                    }
                }
            }
            /* reset and start looking for another token */
            start_bracket_index = -1;
            end_bracket_index = -1;
        }
        /* if we didn't replace a token, then carry on copying input to output */
        if (!token_replaced)
        {
            output[o_index] = input[index];
            o_index++;
        }
    }
    /* terminate the output string */
    output[o_index] = '\0';
}

/*********************************************************************************************************************
 * @brief  wait for key pressed
 *
 * This function wait for key pressed.
 * @param[in]   None
 * @retval      char                    returns key pressed
 *********************************************************************************************************************/
static char Console_WaitForKeypress(void)
{
    uint8_t rx_buf = 0;

    ConsoleUserInputReceived = false;
    /* Read UART data */
    R_SCI_UART_Read (&g_console_uart_ctrl, &rx_buf, 1);
    while (!ConsoleUserInputReceived)
    {
        vTaskDelay(1);
    }

    return ((char) rx_buf);
}

static void Console_StartAppMenu(int8_t selectedMenu)
{
    fsp_err_t err = FSP_SUCCESS;
    int8_t key_pressed = -1;

    Console_ColorPrintf("\r\n[GREEN]Starting AWS cloud Application....[WHITE]\r\n");
    /* Send notify to start aws thread */
    xTaskNotifyFromISR(cloud_app_thread, 1, 1, NULL);

    /* Disable console thread with infinite loop.*/
    while (true)
    {
        vTaskDelay (5000);
    }
}

static void Console_DefaultClaimCredMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;

    Console_ResetClaimCredentials();
    Console_ColorPrintf("[GREEN]\r\nClaim Credentials erased from flash. Default ones will be used.\r\n"
                        "\r\n[YELLOW]Please Restart CloudKit...\r\n");
    while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
    {
        /* Cant recover from that without rereading flash data, thus for simpler setup,
         * ask user to restart app */
        vTaskDelay (5000);
    }
}


static void Console_HelpMenu (int8_t selectedMenu)
{
    int8_t key_pressed = -1;

    /* Print Console_HelpMenu menu content to console */
    Console_ColorPrintf("\r\nHELP\r\n\r\n"
                        "  Type 1 to start Cloud Application\r\n"
                        "  Type 2 to save MQTT broker end point in flash memory\r\n"
                        "  Type 3 to save AWS's claim certificate in flash memory\r\n"
                        "  Type 4 to save AWS's RSA claim private key in flash memory\r\n"
                        "  Type 5 to save IOT thing name in flash memory\r\n"
                        "  Type 6 to check if credentials saved and their validity (format, length, etc.)\r\n"
                        "  Type 7 to get information about flash memory usage of this Renesas Cloud Kit\r\n"
                            CONSOLE_MENU_RETURN);
    while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
    {
        /* Wait for exit key to be pressed to return to main menu */
        key_pressed = Console_WaitForKeypress();
    }
}

static void Console_FlashInfoMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;
    uint32_t totalMem, allocatedMem, freeMem;

    Console_FlashSizesInfo(&totalMem, &allocatedMem, &freeMem);
    Console_ColorPrintf((void *) "\r\n %d) DATA FLASH INFO\r\n", selectedMenu);
    /* Print data flash memory information on console */
    Console_ColorPrintf(" \r\n Physical size:  %ld bytes \r\n Allocated_size: %ld bytes"
                        "\r\n Free_size: %ld bytes" CONSOLE_MENU_RETURN,
                        totalMem, allocatedMem, freeMem);
    while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
    {
        /* Wait for exit key to be pressed to return to main menu */
        key_pressed = Console_WaitForKeypress();
    }
}

static void Console_FlashWriteCertMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;
    char lastParsedChar;
    Console_ColorPrintf((void *) "\r\n %d) DATA FLASH WRITE CLAIM CERTIFICATE\r\n"
                                 "\r\n[ORANGE]Paste claim certificate "
                                 "[WHITE](or press BACKSPACE key to return to menu)\r\n", selectedMenu);
    lastParsedChar = Console_ParseUserCredentials(CONSOLE_CERTIFICATE);
    if(lastParsedChar != CONSOLE_MENU_EXIT_KEY)
    {
        Console_StoreAwsCredential(CONSOLE_CERTIFICATE,ConsoleCredentialBuffer,
                                   strlen(ConsoleCredentialBuffer));
        Console_ColorPrintf(CONSOLE_MENU_RETURN);
        while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
        {
            /* Wait for exit key to be pressed to return to main menu */
            key_pressed = Console_WaitForKeypress();
        }
    }
}

static void Console_FlashWriteKeyMeu (int8_t selectedMenu)
{
    int8_t key_pressed = -1;
    char lastParsedChar;

    Console_ColorPrintf((void *) "\r\n %d) DATA FLASH WRITE RSA CLAIM PRIVATE KEY\r\n"
                                 "\r\n[ORANGE]Paste claim private key "
                                 "[WHITE](or press BACKSPACE key to return to menu)\r\n", selectedMenu);
    lastParsedChar = Console_ParseUserCredentials(CONSOLE_RSA_PRIVATE_KEY);
    if(lastParsedChar != CONSOLE_MENU_EXIT_KEY)
    {
        Console_StoreAwsCredential(CONSOLE_RSA_PRIVATE_KEY, ConsoleCredentialBuffer,
                                   strlen(ConsoleCredentialBuffer));
        Console_ColorPrintf(CONSOLE_MENU_RETURN);
        while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
        {
            /* Wait for exit key to be pressed to return to main menu */
            key_pressed = Console_WaitForKeypress();
        }
    }
}

static void Console_FlashWriteIotNameMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;
    char lastParsedChar;

    Console_ColorPrintf((void *) "\r\n %d) DATA FLASH WRITE IOT THING NAME\r\n"
                                 "\r\nPaste Iot Thing name then press [ORANGE]ENTER "
                                 "[WHITE](or press BACKSPACE key to return to menu)\r\n", selectedMenu);
    lastParsedChar = Console_ParseUserCredentials(CONSOLE_IOT_THING_NAME);
    if(lastParsedChar != CONSOLE_MENU_EXIT_KEY)
    {
        /* Subtract 1 from length because input for IoT name requires user to press ENTER, which is not
         * want in the input buffer*/
        Console_StoreAwsCredential(CONSOLE_IOT_THING_NAME,ConsoleCredentialBuffer,
                                   strlen(ConsoleCredentialBuffer) -1u);
        Console_ColorPrintf(CONSOLE_MENU_RETURN);
        while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
        {
            /* Wait for exit key to be pressed to return to main menu */
            key_pressed = Console_WaitForKeypress();
        }
    }
}

static void Console_FlashWriteEndPointMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;
    char lastParsedChar;

    Console_ColorPrintf((void *) "\r\n %d) DATA FLASH WRITE MQTT BROKER ENDPOINT\r\n"
                                 "\r\nPaste MQTT broker endpoint then press [ORANGE]ENTER "
                                 "[WHITE](or press BACKSPACE key bar to return to menu)\r\n", selectedMenu);
    lastParsedChar = Console_ParseUserCredentials(CONSOLE_MQTT_ENDPOINT);
    if(lastParsedChar != CONSOLE_MENU_EXIT_KEY)
    {
        /* Subtract 1 from length because input for MQTT Broker Endpoint  requires user to press ENTER, which is not
         * want in the input buffer */
        Console_StoreAwsCredential(CONSOLE_MQTT_ENDPOINT,ConsoleCredentialBuffer,
                                   strlen(ConsoleCredentialBuffer) -1u);
        Console_ColorPrintf(CONSOLE_MENU_RETURN);
        while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
        {
            /* Wait for exit key to be pressed to return to main menu */
            key_pressed = Console_WaitForKeypress();
        }
    }
}

static void Console_FlashCheckCredentialsMenu(int8_t selectedMenu)
{
    int8_t key_pressed = -1;

    /* Validate stored credentials */
    Console_ColorPrintf(CONSOLE_FLASH_CHECK_CREDENTIALS);
    Console_CheckStoredCredentials();
    Console_ColorPrintf(CONSOLE_MENU_RETURN);
    while ((CONSOLE_MENU_EXIT_KEY != key_pressed) && (CONSOLE_CONNECTION_ABORT != key_pressed))
    {
        /* Wait for exit key to be pressed to return to main menu */
        key_pressed = Console_WaitForKeypress();
    }
}


static char Console_ParseUserCredentials(ConsoleCredential_t credential)
{
    bool inputCompleted = false;
    char lastCharParsed = 0u;
    char * ptrSubStr = NULL;

    /* Clear global user input buffers */
    memset (ConsoleInputBuffer, 0, TRANSFER_LENGTH);
    ConsoleInputIndex = 0;
    memset (ConsoleCredentialBuffer, 0, TRANSFER_LENGTH);
    ConsoleCredentialIndex = 0;

    /* Enable storing of incoming data on UART to ConsoleCredentialBuffer */
    ConsoleReadCredential = true;
    while (1)
    {
        /* This while loop does not catch every new character. There might be many char in the
         * buffer when this is executed. Therefore, look for substrings in the buffer until
         * a meaningful pattern is recognized */
        switch (credential)
        {
            case CONSOLE_CERTIFICATE:
                ptrSubStr = strstr (ConsoleCredentialBuffer, CONSOLE_END_OF_CERTIFICATE_SUBSTR);
                if(ptrSubStr != NULL)
                {
                    /* go back one character to check if its an EOL char */
                    if(*(ptrSubStr -1u) == '\n')
                    {
                        inputCompleted = true;
                    }
                }
                break;
            case CONSOLE_RSA_PRIVATE_KEY:
                ptrSubStr = strstr (ConsoleCredentialBuffer, CONSOLE_END_OF_RSA_PRIVATE_KEY_SUBSTR);
                if(ptrSubStr != NULL)
                {
                    inputCompleted = true;
                }
                break;
            case CONSOLE_MQTT_ENDPOINT:
            case CONSOLE_IOT_THING_NAME:
                if(strchr(ConsoleCredentialBuffer, CONSOLE_ENTER_KEY))
                {
                    inputCompleted = true;
                }
                break;
            default:
                /* Should not get here, that is a development error */
                break;
        }

        if(strchr(ConsoleCredentialBuffer, CONSOLE_MENU_EXIT_KEY))
        {
            /* User entered command to exit menu, thus complete input and return exit menu key
             * to tell caller that menu exited without completing requested input */
            inputCompleted = true;
            lastCharParsed = CONSOLE_MENU_EXIT_KEY;
        }

        if(inputCompleted)
        {
            ConsoleReadCredential = false;
            break;
        }
        vTaskDelay (1);
    }
    return lastCharParsed;
}


/*****************************************************************************
 * Function Name: Console_ColorPrintf
 *                As printf, but replaces colour tags with escape codes
 * @param[in] char *format : the format string
 * @param[in] ... : argument list, 0 or more parameters
 * @retval None
 ******************************************************************************/
void Console_ColorPrintf(const char *format, ...)
{
    va_list arglist;
    fsp_err_t err = FSP_SUCCESS;
    static char colour_format[PRINT_BUFFER];
    static char final_buffer[PRINT_BUFFER];

    /* Check if we can obtain the mutex.  If the mutex is not available
     * wait 10 ticks to see if it becomes free.*/
    if ((g_console_out_mutex != NULL) &&
        (xSemaphoreTakeRecursive( g_console_out_mutex, ( TickType_t ) 10 ) == pdTRUE))
    {
        /*We were able to obtain the mutex and can now access the
         * shared resource.*/
        va_start(arglist, format);
        /* replace colour tokens with terminal colour escape codes */
        Console_Detokenise(format, colour_format);
        vsprintf (final_buffer, colour_format, arglist);
        va_end(arglist);

        ConsoleTransmitDone = false;
        /* Uart write data */
        err = R_SCI_UART_Write (&g_console_uart_ctrl, (uint8_t*) final_buffer, strlen (final_buffer));
        assert(FSP_SUCCESS == err);

        while (ConsoleTransmitDone != true)
        {
            vTaskDelay(1);
        }
        /* Release mutex so it can be taken by other tasks */
        xSemaphoreGiveRecursive(g_console_out_mutex);
    }
}

/*********************************************************************************************************************
 * @brief  Callback from driver
 *
 * @param[in]   p_args                  Pointer to callback parameters
 * @retval
 *********************************************************************************************************************/
void Console_UartCallback(uart_callback_args_t *p_args)
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            /* Only put the next character in the receive buffer if there is space for it */
            if (sizeof(ConsoleInputBuffer) > ConsoleInputIndex)
            {
                /* Write either the next one or two bytes depending on the receive data size */
                if (UART_DATA_BITS_8 >= g_console_uart_cfg.data_bits)
                {
                    ConsoleInputBuffer[ConsoleInputIndex] = (uint8_t) p_args->data;
                    if (ConsoleReadCredential == true)
                    {
                        /* Fill received data to write buffer when AWS credentials are being saved */
                        ConsoleCredentialBuffer[ConsoleCredentialIndex++] = (char) ConsoleInputBuffer[ConsoleInputIndex];
                    }
                    ConsoleInputIndex++;
                }
                else
                {
                    uint16_t *p_dest = (uint16_t*) &ConsoleInputBuffer[ConsoleInputIndex];
                    *p_dest = (uint16_t) p_args->data;
                    ConsoleInputIndex += 2;
                }

            }
            break;
        }
            /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
        {
            ConsoleUserInputReceived = true;
            break;
        }
            /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            ConsoleTransmitDone = true;
            break;
        }
        default:
        {
        }
    }
}


/*******************************************************************************************************************//**
 * @brief       Initialize  UART.
 * @param[in]   None
 * @retval      FSP_SUCCESS         Upon successful open and start of timer
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 ***********************************************************************************************************************/
void Console_Init(void)
{
    fsp_err_t err;

    /* Initialize UART channel with baud rate 115200 */
    err = R_SCI_UART_Open (&g_console_uart_ctrl, &g_console_uart_cfg);
    assert(err == FSP_SUCCESS);

    /* Open Flash_HP */
    err = R_FLASH_HP_Open(&user_flash_ctrl, &user_flash_cfg);
    if (FSP_SUCCESS == err)
    {
        /* Load credential storage status from flash memory */
         Console_LoadCredentialsFromFlash();

        /* version get API for FLEX pack information */
        R_FSP_VersionGet(&ConsoleFSPversion);

        ConsoleDeviceUUID = R_BSP_UniqueIdGet ();
    }
    else
    {
        APP_ERR_PRINT("\r\n R_FLASH_HP_Open API failed");
        APP_ERR_TRAP(err);
    }
}

void Console_DisplayMenu(void)
{
    int8_t key_pressed = -1;
    int8_t menuItemCount = 0;
    char consoleBanner[1024u];
    uint8_t rx_buf = 0;

    /* Format Application Banner */
    sprintf(consoleBanner, (void *) CONSOLE_CLEAR_SCREEN CONSOLE_CURSOR_HOME BANNER_INFO,
            AP_VERSION, ConsoleFSPversion.version_id_b.major,
            ConsoleFSPversion.version_id_b.minor, ConsoleFSPversion.version_id_b.patch,
            (uint32_t) ConsoleDeviceUUID->unique_id_words[0], (uint32_t) ConsoleDeviceUUID->unique_id_words[1],
            (uint32_t) ConsoleDeviceUUID->unique_id_words[2], (uint32_t) ConsoleDeviceUUID->unique_id_words[3]);
    Console_ColorPrintf(consoleBanner);


    Console_ColorPrintf("\r\n[ORANGE] Press BACKSPACE key to open menu...[WHITE]\r\n");
    /* Give possibility to user to avoid automatic connection with credentials stored in flash.
     * Allow a 1 sec window where user can press BACKSPACE key to avoid starting cloud app and display menu*/
    R_SCI_UART_Read (&g_console_uart_ctrl, &rx_buf, 1);
    vTaskDelay(3000);

    if(rx_buf != CONSOLE_MENU_EXIT_KEY)
    {
        /* User did NOT press BACKSPACE key, thus try to connect with to MQTT broker with credential stored in flash. */
        Console_ColorPrintf("\r\n[GREEN]Starting AWS cloud Application....[WHITE]\r\n");
        /* Give some feedback to user on credentials used */
        if(ConsoleClaimCertificateStored == true)
        {
            Console_ColorPrintf("\r\n[YELLOW]Loading Claim Certificate from flash memory instead of default.[WHITE]\r\n");
        }
        if(ConsoleClaimPrivateKeyStored == true)
        {
            Console_ColorPrintf("\r\n[YELLOW]Loading Claim PrivateKey from flash memory instead of default.[WHITE]\r\n");
        }
        if(ConsoleMqttEndpointStored == true)
        {
            Console_ColorPrintf("\r\n[YELLOW]Loading MQTT Broker Endpoint from flash memory instead of default.[WHITE]\r\n");
        }
        /* Notify cloud app thread so that it can try to use stored credentials to connect the device */
        xTaskNotifyGive( cloud_app_thread );
        /* Stop this thread's execution. If the mqtt broker endpoint exists in flash */
        while(true)
        {
            vTaskDelay(5000);
        }
    }

    /* Wait user inputs an option available on menu OR until uart is disconnected */
    while (CONSOLE_CONNECTION_ABORT != key_pressed)
    {
        static bool optionValid = true;
        /* Print banner info */
        Console_ColorPrintf(consoleBanner);
        Console_ColorPrintf(CONSOLE_SUB_OPTIONS);
        for (int8_t item = 0; NULL != ConsoleMainMenus[item].displayMenu; item++ )
        {
            Console_ColorPrintf((void *) "\r\n %d. %s", (item + 1), ConsoleMainMenus[menuItemCount++].menuName);
        }
        Console_ColorPrintf("\r\n\r\n> Enter (1-%d) to select options\r\n\r\n", menuItemCount);

        if(optionValid == false)
        {
            Console_ColorPrintf("\r\n[ORANGE]Please enter valid option.[WHITE]");
        }

        key_pressed = Console_WaitForKeypress();

        /* Translate menu number from ascii to int */
        key_pressed = (int8_t) (key_pressed - '0');

        if ((key_pressed > 0) && (key_pressed <= menuItemCount))
        {
            optionValid = true;
            Console_ColorPrintf(consoleBanner);
            ConsoleMainMenus[key_pressed - 1].displayMenu(key_pressed);
        }
        else
        {
            optionValid = false;
        }
        menuItemCount = 0u;
        vTaskDelay(100);
    }
}