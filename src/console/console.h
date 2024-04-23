/***********************************************************************************************************************
 * File Name    : console.h
 * Description  : Contains data structures and function declarations for console
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
#ifndef CONSOLE_CONSOLE_H_
#define CONSOLE_CONSOLE_H_

/* generic headers */
#include <logging_levels.h>
#include <SEGGER_RTT.h>
#include <console_thread.h>



#define MINIMUM_TOKEN_LENGTH      ( 5)   // minimum length of a colour token
#define MAXIMUM_TOKEN_LENGTH      (10)   // maximum length of a colour token
#define PRINT_BUFFER              (2*1024)
#define TRANSFER_LENGTH           (2048)




#define RTT_TERMINAL  (1)
#define ITM_TERMINAL  (2)
#define UART_TERMINAL (3)

#if 0
#define LOG_TERMINAL      (RTT_TERMINAL)       /* error conditions   */
#else
#define LOG_TERMINAL      (UART_TERMINAL)     /* error conditions   */
#endif

#define LOG_LVL      (LOG_INFO)       /* error conditions   */


#define RESET_VALUE             (0x00)
#define BUFFER_LENGTH_SHORT      (192)

#define KIT_NAME                "CK-RA6M5"

#define SEGGER_INDEX            (0)


#define APP_PRINT(fn_, ...)         ({if(LOG_TERMINAL == RTT_TERMINAL){\
                                       SEGGER_RTT_printf (SEGGER_INDEX,(fn_), ##__VA_ARGS__);\
                                    }\
                                    else {                             \
                                       Console_ColorPrintf("[WHITE]"); \
                                        Console_ColorPrintf(fn_ , ##__VA_ARGS__);\
                                    }})

#define APP_ERR_PRINT(fn_, ...)     ({if(LOG_LVL >= LOG_ERROR){\
                                        if(LOG_TERMINAL == RTT_TERMINAL){\
                                            SEGGER_RTT_printf (SEGGER_INDEX, "[ERR] In Function: %s(), ", __FUNCTION__);\
      									    SEGGER_RTT_printf (SEGGER_INDEX, (fn_), ##__VA_ARGS__);\
                                     	}\
                                        else {\
                                            Console_ColorPrintf("[RED][ERR] In Function: %s(), ",__FUNCTION__);\
                                            Console_ColorPrintf(fn_, ##__VA_ARGS__);\
                                        }\
								     }})

#define APP_WARN_PRINT(fn_, ...) ({if(LOG_LVL >= LOG_WARN){\
                                     if(LOG_TERMINAL == RTT_TERMINAL){\
                                         SEGGER_RTT_printf (SEGGER_INDEX, "[WARN] In Function: %s(), ", __FUNCTION__);\
                                         SEGGER_RTT_printf (SEGGER_INDEX, (fn_), ##__VA_ARGS__);\
                                     	}\
                                     else {\
                                         Console_ColorPrintf("[YELLOW][WARN] In Function: %s(), ",__FUNCTION__); \
                                         Console_ColorPrintf(fn_, ##__VA_ARGS__);\
                                     }\
								 }})

#define APP_INFO_PRINT(fn_, ...) ({if(LOG_LVL >= LOG_INFO){\
                                     if(LOG_TERMINAL == RTT_TERMINAL){\
                                         SEGGER_RTT_printf (SEGGER_INDEX, "[INFO] In Function: %s(), ", __FUNCTION__);\
                                         SEGGER_RTT_printf (SEGGER_INDEX, (fn_), ##__VA_ARGS__);\
                                     	}\
                                     else {\
                                         Console_ColorPrintf("[WHITE][INFO] In Function: %s(), ",__FUNCTION__);\
                                         Console_ColorPrintf(fn_, ##__VA_ARGS__);\
                                    }\
                                 }})

#define APP_DBG_PRINT(fn_, ...)  ({if(LOG_LVL >= LOG_DEBUG){\
                                     if(LOG_TERMINAL == RTT_TERMINAL){\
                                         SEGGER_RTT_printf (SEGGER_INDEX, "[DBG] In Function: %s(), ", __FUNCTION__);\
                                         SEGGER_RTT_printf (SEGGER_INDEX, (fn_), ##__VA_ARGS__);\
                                     	}\
                                     else {\
                                         Console_ColorPrintf("[BLUE][DBG] In Function: %s(), ",__FUNCTION__); \
                                         Console_ColorPrintf(fn_, ##__VA_ARGS__);\
                                     }\
                                 }})

#define APP_ERR_TRAP(err)        ({if(err){\
                                     if(LOG_LVL >= RTT_TERMINAL){\
                                         SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", (unsigned int)err);\
                                         __BKPT(0);\
                                     }\
                                     else {\
                                         Console_ColorPrintf("[ORANGE]\r\nReturned Error Code: 0x%x	\r\n", (unsigned int)err);\
                                         __BKPT(0);\
                                     }\
                                 }})

#define APP_READ(read_data)     (SEGGER_RTT_Read (SEGGER_INDEX, read_data, sizeof(read_data)))

#define APP_CHECK_DATA          (SEGGER_RTT_HasKey())

void Console_ColorPrintf(const char *format, ...);
void Console_Init(void);
void Console_DisplayMenu(void);

extern TaskHandle_t console_thread;

#endif /* CONSOLE_CONSOLE_H_ */
