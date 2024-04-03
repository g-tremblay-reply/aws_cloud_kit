/***********************************************************************************************************************
 * File Name    : sensor_hs3001.c
 * Description  : Contains data structures and function definitions for HS3001 sensor
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
#include <sensor_hs3001.h>
#include <console.h>
#include <sensor_thread.h>

typedef enum
{
    SENSOR_HS3001_START_MEASUREMENT,
    SENSOR_HS3001_WAIT_START_MEASUREMENT_NOTIF,
    SENSOR_HS3001_WAIT_END_OF_MEASUREMENT,
    SENSOR_HS3001_WAIT_READ_RESULT_NOTIF,
    SENSOR_HS3001_READ_RESULT,
    SENSOR_HS3001_CALCULATE_DATA,
} Sensor_Hs3001State_t; //todo implement state machine with this

/* Utilitfy MACRO to loop 50ms delays until expression becomes alse*/
#define WAIT_WHILE_FALSE(e) ({while(!(e)) { vTaskDelay (50); }})

/* Variable declarations*/
static volatile fsp_err_t g_err; /* FSP Error variable*/
volatile bool g_hs300x_completed = false;
static float SensorHs3001Humidity;
static float SensorHs3001Temperature;

/*******************************************************************************************************************//**
 * @brief       Quick sensor setup for HS3001
 * @param[in]
 * @retval
 * @retval
 ***********************************************************************************************************************/
void Sensor_Hs3001Init(void)
{
    fsp_err_t err;
    /* Open HS300X sensor instance, this must be done before calling any HS300X API */
    err = g_hs300x_sensor0.p_api->open (g_hs300x_sensor0.p_ctrl, g_hs300x_sensor0.p_cfg);
    if (err != FSP_SUCCESS)
    {
        APP_DBG_PRINT("\r\nHS3001 sensor setup failed:%d\r\n", err);
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("\r\nHS3001 sensor setup success\r\n");
    }

}

void Sensor_Hs3001MainFunction(void)
{
    rm_hs300x_raw_data_t TH_rawdata;
    rm_hs300x_data_t hs3001Data;

    /* start measurement */ //TODO fix this aweful runtime with a state machine
    g_hs300x_completed = false;

    g_err = g_hs300x_sensor0.p_api->measurementStart (g_hs300x_sensor0.p_ctrl);
    assert(FSP_SUCCESS == g_err);
    WAIT_WHILE_FALSE(g_hs300x_completed);

    /* Keep attempting to read the data until it stabilises.
     * Section 6.6 of the HS3001 datasheet stipulates a range of
     * measurement times with corresponding to the range of
     * sensor resolutions. */
    do
    {
        /* read the data */
        g_hs300x_completed = false;

        g_err = g_hs300x_sensor0.p_api->read (g_hs300x_sensor0.p_ctrl, &TH_rawdata);
        assert(FSP_SUCCESS == g_err);

        WAIT_WHILE_FALSE(g_hs300x_completed);

        /* Calculate results */
        g_err = g_hs300x_sensor0.p_api->dataCalculate (g_hs300x_sensor0.p_ctrl, &TH_rawdata, &hs3001Data);
        assert((FSP_SUCCESS == g_err) || (g_err == FSP_ERR_SENSOR_INVALID_DATA));
    }
    while (g_err == FSP_ERR_SENSOR_INVALID_DATA);

    /* Update sensor data. Disable interrupts in case there is a context switch
     * with read access to those global variables through the GetData api */
    SensorHs3001Humidity = (float) hs3001Data.humidity.integer_part
            + (float) hs3001Data.humidity.decimal_part * 0.01F;
    SensorHs3001Temperature = (float) hs3001Data.temperature.integer_part
            + (float) hs3001Data.temperature.decimal_part * 0.01F;
}

void Sensor_Hs3001GetData(float *temperature, float *humidity)
{
    *temperature = SensorHs3001Temperature;
    *humidity = SensorHs3001Humidity;
}

/*******************************************************************************************************************//**
 * @brief                  HS3001 sensor callback
 * @param[in]   p_args     HS3001 callback event
 * @retval
 * @retval
 ***********************************************************************************************************************/
void Sensor_Hs3001Callback(rm_hs300x_callback_args_t *p_args)
{
    if (RM_HS300X_EVENT_SUCCESS == p_args->event)
    {
        g_hs300x_completed = true;
    }
    else if (RM_HS300X_EVENT_ERROR == p_args->event)
    {
        APP_DBG_PRINT("\r\nRM_HS300X_EVENT_ERROR\r\n");
    }
    else
    {
        /* Do nothing */
    }
    FSP_PARAMETER_NOT_USED(p_args);
}

