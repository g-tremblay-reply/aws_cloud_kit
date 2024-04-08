/***********************************************************************************************************************
 * File Name    : sensor_iaq.c
 * Description  : Contains data structures and function definitions
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
#include <sensor_iaq.h>


typedef enum
{
    SENSOR_IAQ_START_MEASUREMENT,
    SENSOR_IAQ_WAIT_START_MEASUREMENT_NOTIF,
    SENSOR_IAQ_WAIT_END_OF_MEASUREMENT,
    SENSOR_IAQ_WAIT_READ_RESULT_NOTIF,
    SENSOR_IAQ_READ_RESULT,
    SENSOR_IAQ_CALCULATE_DATA,
} Sensor_IaqState_t;


static rm_zmod4xxx_raw_data_t SensorIaqRawData;
static rm_zmod4xxx_iaq_1st_data_t SensorIaqData;
static Sensor_IaqState_t SensorIaqMeasurementState = SENSOR_IAQ_START_MEASUREMENT;

void SensorIaq_Init(void)
{
    fsp_err_t err;

    /* Open ZMOD4XXX sensor instance, this must be done before calling any ZMOD4XXX API */
    err = g_zmod4xxx_sensor0.p_api->open (g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);
    if (err != FSP_SUCCESS)
    {
        APP_DBG_PRINT("\r\nZMOD4410 sensor setup failed:%d\r\n", err);
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("\r\nZMOD4410 sensor setup success\r\n");
    }
}

void SensorIaq_CommCallback(rm_zmod4xxx_callback_args_t *p_args)
{
    switch(SensorIaqMeasurementState)
    {
        case SENSOR_IAQ_WAIT_START_MEASUREMENT_NOTIF:
            if ((RM_ZMOD4XXX_EVENT_SUCCESS == p_args->event) ||
                (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event))
            {
                SensorIaqMeasurementState = SENSOR_IAQ_WAIT_END_OF_MEASUREMENT;
            }
            else
            {
                /* Underlying driver error, retry sending frame to start measurement */
                SensorIaqMeasurementState = SENSOR_IAQ_START_MEASUREMENT;
            }
            break;
        case SENSOR_IAQ_WAIT_READ_RESULT_NOTIF:
            if ((RM_ZMOD4XXX_EVENT_SUCCESS == p_args->event) ||
                (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event))
            {
                /* Use the raw data to calculate IAW sensor data on next MainFunction iteration */
                SensorIaqMeasurementState = SENSOR_IAQ_CALCULATE_DATA;
            }
            else
            {
                /* Underlying driver error, retry sendding frame to read result */
                SensorIaqMeasurementState = SENSOR_IAQ_READ_RESULT;
            }
            break;
        default:
            //TODO log dev error
            break;
    }
}

void SensorIaq_EndOfMeasurement(rm_zmod4xxx_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    if(SensorIaqMeasurementState == SENSOR_IAQ_WAIT_END_OF_MEASUREMENT)
    {
        /* Got notification of end of measurement. On next MainFunction iteration, try reading results */
        SensorIaqMeasurementState = SENSOR_IAQ_READ_RESULT;
    }
}

void SensorIaq_MainFunction(void)
{
    fsp_err_t err;

    switch (SensorIaqMeasurementState)
    {
        case SENSOR_IAQ_START_MEASUREMENT:
        {
            /* Start measurement */
            err = g_zmod4xxx_sensor0.p_api->measurementStart (g_zmod4xxx_sensor0.p_ctrl);
            if (FSP_SUCCESS == err)
            {
                SensorIaqMeasurementState = SENSOR_IAQ_WAIT_START_MEASUREMENT_NOTIF;
            }
            else
            {
                APP_DBG_PRINT("\r\nTask zmod4410 task error in seq 1:%d\r\n", err);
            }
            break;
        }
        case SENSOR_IAQ_READ_RESULT:
        {
            /* Read data */
            err = g_zmod4xxx_sensor0.p_api->read (g_zmod4xxx_sensor0.p_ctrl, &SensorIaqRawData);
            if (FSP_SUCCESS == err)
            {
                SensorIaqMeasurementState = SENSOR_IAQ_WAIT_READ_RESULT_NOTIF;
            }
            break;
        }
        case SENSOR_IAQ_CALCULATE_DATA:
        {
            /* Calculate data. Disable interrupts to update data in case context switch happens and public api
             * GetData is called to access the SensorIaqData */
            portENTER_CRITICAL();
            g_zmod4xxx_sensor0.p_api->iaq1stGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl,
                                                             &SensorIaqRawData,
                                                             &SensorIaqData);
            portEXIT_CRITICAL();
            SensorIaqMeasurementState = SENSOR_IAQ_START_MEASUREMENT;
            break;
        }
        default:
        {
            APP_DBG_PRINT("\r\nTask zmod4410 task delete in default case:%d\r\n", err);
        }
        break;
    }
}

void SensorIaq_GetData(rm_zmod4xxx_iaq_1st_data_t *data)
{
    *data = SensorIaqData;
}

