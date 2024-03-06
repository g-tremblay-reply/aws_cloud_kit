/***********************************************************************************************************************
 * File Name    : RA_ICM20948.c
 * Description  : Contains function definitions for ICM20948 sensor
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
#include "icm.h"
#include "sensor/ICM_20948.h"
#include "usr_data.h"

extern xyzFloat corrAccRaw; // @suppress("Global (API or Non-API) variable prefix")
extern xyzFloat gVal;
extern xyzFloat magValue;
usr_icm_data_t g_icm_data;

/*******************************************************************************************************************//**
 * @brief   Send ICM data to the queue
 * @param[in]   None
 * @retval      None
 ***********************************************************************************************************************/
void send_icm_data_to_queue(void)
{
    /* Get value of ICM sensor */
    ICM_20948_get();
    
    /* Update value for icm_data variable */
    g_icm_data.acc_data.x = corrAccRaw.x;
    g_icm_data.acc_data.y = corrAccRaw.y;
    g_icm_data.acc_data.z = corrAccRaw.z;
    g_icm_data.gyr_data.x = gVal.x;
    g_icm_data.gyr_data.y = gVal.y;
    g_icm_data.gyr_data.z = gVal.z;
    g_icm_data.mag_data.x = magValue.x;
    g_icm_data.mag_data.y = magValue.y;
    g_icm_data.mag_data.z = magValue.z;

    xQueueOverwrite(g_icm_queue, &g_icm_data);
}

