/***********************************************************************************************************************
 * File Name    : console_thread_entry.c
 * Description  : Contains data structures and functions used in Console related application
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas FSP license agreement. Unless otherwise agreed in an FSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/
#include "common_data.h"
#include "console.h"
#include "console_thread.h"

 extern TaskHandle_t console_thread;

 /**************************************************************************************
  * @brief Console Thread entry function
  * @param[in]  pvParameters contains TaskHandle_t
  * @retval
  **************************************************************************************/


void console_thread_entry(void *pvParameters)
 {
	FSP_PARAMETER_NOT_USED(pvParameters);
    Console_Init();

    /* wait sensor thread to init of sensors */
    xTaskNotifyWait(pdFALSE, pdFALSE, (uint32_t* )&console_thread, portMAX_DELAY);
	while (1)
	{
        Console_DisplayMenu();
	}
}
