/***********************************************************************************************************************
 * File Name    : sensor_oaq.c
 * Description  : Contains data structures and function definitions for ZMOD4510 sensor data read
 ***********************************************************************************************************************/

#include <sensor_oaq.h>
#include <sensor_thread.h>
#include <console.h>

typedef enum
{
    SENSOR_OAQ_START_MEASUREMENT,
    SENSOR_OAQ_WAIT_START_MEASUREMENT_NOTIF,
    SENSOR_OAQ_WAIT_END_OF_MEASUREMENT,
    SENSOR_OAQ_WAIT_READ_RESULT_NOTIF,
    SENSOR_OAQ_READ_RESULT,
    SENSOR_OAQ_CALCULATE_DATA,
} Sensor_OaqState_t;

static Sensor_OaqState_t SensorOaqMeasurementState = SENSOR_OAQ_START_MEASUREMENT;
static rm_zmod4xxx_raw_data_t SensorOaqRawData;
static rm_zmod4xxx_oaq_1st_data_t SensorOaqData;


void Sensor_OaqInit(void)
{
    fsp_err_t err;

    /* Open ZMOD4XXX sensor instance, this must be done before calling any ZMOD4XXX API */
    err = g_zmod4xxx_sensor1.p_api->open (g_zmod4xxx_sensor1.p_ctrl, g_zmod4xxx_sensor1.p_cfg);
    if (err != FSP_SUCCESS)
    {
        APP_DBG_PRINT("\r\nZMOD4510 sensor setup failed:%d\r\n", err);
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_PRINT("\r\nZMOD4510 sensor setup success\r\n");
    }
}

void Sensor_OaqCommCallback(rm_zmod4xxx_callback_args_t *p_args)
{
    switch(SensorOaqMeasurementState)
    {
        case SENSOR_OAQ_WAIT_START_MEASUREMENT_NOTIF:
            if ((RM_ZMOD4XXX_EVENT_SUCCESS == p_args->event) ||
                (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event))
            {
                SensorOaqMeasurementState = SENSOR_OAQ_WAIT_END_OF_MEASUREMENT;
            }
            else
            {
                /* Underlying driver error, retry sending frame to start measurement */
                SensorOaqMeasurementState = SENSOR_OAQ_START_MEASUREMENT;
            }
            break;
        case SENSOR_OAQ_WAIT_READ_RESULT_NOTIF:
            if ((RM_ZMOD4XXX_EVENT_SUCCESS == p_args->event) ||
                (RM_ZMOD4XXX_EVENT_MEASUREMENT_COMPLETE == p_args->event))
            {
                /* Use the raw data to calculate IAW sensor data on next MainFunction iteration */
                SensorOaqMeasurementState = SENSOR_OAQ_CALCULATE_DATA;
            }
            else
            {
                /* Underlying driver error, retry sending frame to read result */
                SensorOaqMeasurementState = SENSOR_OAQ_READ_RESULT;
            }
            break;
        default:
            //TODO log dev error
            break;
    }
}

void Sensor_OaqEndOfMeasurement(rm_zmod4xxx_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    if(SensorOaqMeasurementState == SENSOR_OAQ_WAIT_END_OF_MEASUREMENT)
    {
        /* Got notification of end of measurement. On next MainFunction iteration, try reading results */
        SensorOaqMeasurementState = SENSOR_OAQ_READ_RESULT;
    }
}


void Sensor_OaqMainFunction(void)
{
    fsp_err_t err;

    switch (SensorOaqMeasurementState)
    {
        case SENSOR_OAQ_START_MEASUREMENT:
        {
            /* Start measurement */
            err = g_zmod4xxx_sensor1.p_api->measurementStart (g_zmod4xxx_sensor1.p_ctrl);
            if (FSP_SUCCESS == err)
            {
                SensorOaqMeasurementState = SENSOR_OAQ_WAIT_START_MEASUREMENT_NOTIF;
            }
            else
            {
                APP_DBG_PRINT("\r\nTask zmod4510 task measurement failed in seq 1:%d\r\n", err);
            }
            break;
        }
        case SENSOR_OAQ_READ_RESULT:
        {
            /* Read data */
            err = g_zmod4xxx_sensor1.p_api->read (g_zmod4xxx_sensor1.p_ctrl, &SensorOaqRawData);
            if (FSP_SUCCESS == err)
            {
                SensorOaqMeasurementState = SENSOR_OAQ_WAIT_READ_RESULT_NOTIF;
            }
            break;
        }
        case SENSOR_OAQ_CALCULATE_DATA:
        {
            /* Calculate data. Disable interrupts to update data in case context switch happens and public api
             * GetData is called to access the SensorIaqData */
            portENTER_CRITICAL();
            g_zmod4xxx_sensor1.p_api->oaq1stGenDataCalculate (g_zmod4xxx_sensor1.p_ctrl,
                                                                    &SensorOaqRawData,
                                                                    &SensorOaqData);
            portEXIT_CRITICAL();
            SensorOaqMeasurementState = SENSOR_OAQ_START_MEASUREMENT;
        }
        break;

        default:
        {
            APP_DBG_PRINT("\r\nTask zmod4510 task delete in default case:%d\r\n", err);
        }
        break;
    }
}

void Sensor_OaqGetData(float *data)
{
    *data = SensorOaqData.aiq;
}