/***********************************************************************************************************************
 * File Name    : RA_ICP10101.c
 * Description  : Contains function definitions for ICP10101 sensor
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
#include "sensor/icp.h"
#include "sensor/ICP_10101.h"
#include "usr_data.h"

extern float Temperature; // @suppress("Global (API or Non-API) variable prefix")
extern float Pressure;
usr_icp_data_t g_icp_data;

/*******************************************************************************************************************//**
 * @brief   Send ICP data to the queue
 * @param[in]   None
 * @retval      None
 ***********************************************************************************************************************/
void send_icp_data_to_queue(void)
{
    /* Get value of ICP sensor */
    ICP_10101_get();
    
    /* Update value for icp_data variable */
    g_icp_data.temperature_C = Temperature;
    g_icp_data.pressure_Pa = Pressure;

    xQueueOverwrite(g_icp_queue, &g_icp_data);
}

