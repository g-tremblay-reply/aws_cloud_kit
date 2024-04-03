/***********************************************************************************************************************
 * File Name    : sensor_ob1203.c
 * Description  : Contains data structure and function definitions of OB1203 sensor data calculation
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

#include "sensor_ob1203.h"
#include <oximeter_thread.h>


typedef enum
{
    SENSOR_OB1203_INIT,
    SENSOR_OB1203_START_MEASUREMENT,
    SENSOR_OB1203_WAIT_MEASUREMENT,
    SENSOR_OB1203_CHECK_MODE_SWITCH,
    SENSOR_OB1203_READ_DATA,
    SENSOR_OB1203_CALCULATE_PPG,
    SENSOR_OB1203_AUTO_GAIN_CURRENT_CONTROL,
    SENSOR_OB1203_CHECK_ALGO,
    SENSOR_OB1203_ADD_PPG_SAMPLES,
    SENSOR_OB1203_CALCULATE_HR_SP02,
    SENSOR_OB1203_CALCULATE_RESPIRATION_RATE,
    SENSOR_OB1203_CHECK_PERFUSION_INDEX,
} Sensor_Ob1203State_t;

/* Static variables */
static spo2_t gs_spo2;
static volatile ob1203_bio_data_t SensorOb1203data;

bool result;
rm_ob1203_raw_data_t g_raw_data;
rm_ob1203_ppg_data_t ppg_data;
ob1203_bio_t ob1203_bio;
bool change = false;
bool valid = false;
bool update = false;
bool ready = false;
ob1203_bio_gain_currents_t gain_currents;

static void print_ob_data(void);


/***********************************************************************************************************************
 * @brief       Print ob1203's data.
 * @param       None
 * @retval      None
 **********************************************************************************************************************/
static void print_ob_data(void)
{
#if _OB1203_SENSOR_ENABLE_
    APP_DBG_PRINT("\r\n SPO2 : %d  ", SensorOb1203data.spo2);
    APP_DBG_PRINT(" Heart rate : %d  ", SensorOb1203data.heart_rate);
    APP_DBG_PRINT(" breath rate : %d  ", SensorOb1203data.respiration_rate);
    APP_DBG_PRINT("perfusion_index : %d \r\n ",SensorOb1203data.perfusion_index);
#endif
}


/***********************************************************************************************************************
 * @brief       Initialization of ob1203 sensor
 * @param       None
 * @retval      None        
 **********************************************************************************************************************/
void Sensor_Ob1203Init(void)
{
    /* Set default gain and currents */
    gain_currents.gain.ppg_prox = g_ob1203_sensor0_extended_cfg.ppg_prox_gain;
    gain_currents.currents.ir_led = g_ob1203_sensor0_extended_cfg.ppg_ir_led_current;
    gain_currents.currents.red_led = g_ob1203_sensor0_extended_cfg.ppg_red_led_current;

    /* Open OB1203 Bio extension */
    result = ob1203_bio_open(&ob1203_bio,
                             (rm_ob1203_instance_t*)&g_ob1203_sensor1, // Proximity mode
                             (rm_ob1203_instance_t*)&g_ob1203_sensor0,  // PPG mode
                             &gs_spo2);
    if (false == result)
    {
    	APP_PRINT ("\r\nOB1203 sensor setup fail \r\n");
    }
    else
    {
    	APP_PRINT ("\r\nOB1203 sensor setup success \r\n");
    }

}

/***********************************************************************************************************************
 * @brief       Main activities of ob1203 sensor    
 * @param       None
 * @retval      None        
 **********************************************************************************************************************/
void Sensor_Ob1203MainFunction(void)
{
    static Sensor_Ob1203State_t measurementState = SENSOR_OB1203_INIT;

    switch (measurementState)
    {
        case SENSOR_OB1203_INIT :
        {
            /* Initialize an operation mode */
            result = ob1203_bio_operation_mode_init(&ob1203_bio);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_operation_mode_init failed\r\n");
            }

            measurementState = SENSOR_OB1203_START_MEASUREMENT;
        }
        break;

        case SENSOR_OB1203_START_MEASUREMENT :
        {
            /* Start a measurement */
            result = ob1203_bio_measurement_start(&ob1203_bio);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_measurement_start failed\r\n");
            }

            measurementState = SENSOR_OB1203_WAIT_MEASUREMENT;
        }
        break;

        case SENSOR_OB1203_WAIT_MEASUREMENT:
        {
            /* Wait measurement period */
            result = ob1203_bio_measurement_period_wait(&ob1203_bio);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_measurement_period_wait failed\r\n");
            }

            measurementState = SENSOR_OB1203_CHECK_MODE_SWITCH;
        }
        break;

        case SENSOR_OB1203_CHECK_MODE_SWITCH :
        {
            /* Check if an operation mode needs to be changed */
            result = ob1203_bio_mode_change_check(&ob1203_bio, &change);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_mode_change_check failed\r\n");
            }

            if (false != change)
            {
                /* Stop the measurement */
                result = ob1203_bio_measurement_stop(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_measurement_stop failed\r\n");
                }

                /* Change to another mode */
                measurementState = SENSOR_OB1203_INIT;
            }
            else
            {
                /* No change */
                measurementState = SENSOR_OB1203_READ_DATA;
            }
        }
        break;

        case SENSOR_OB1203_READ_DATA:
        {
            /* Read raw data */
            result = ob1203_bio_ppg_raw_data_read(&ob1203_bio, &g_raw_data);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_ppg_raw_data_read failed\r\n");
            }

            measurementState = SENSOR_OB1203_CALCULATE_PPG;
        }
        break;

        case SENSOR_OB1203_CALCULATE_PPG:
        {
            /* Calculate PPG data from raw data */
            result = ob1203_bio_ppg_data_calculate(&ob1203_bio, &g_raw_data, &ppg_data, &valid);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_ppg_data_calculate failed\r\n");
            }

            if (false != valid)
            {
                /* Valid data */
                measurementState = SENSOR_OB1203_AUTO_GAIN_CURRENT_CONTROL;
            }
            else
            {
                /* Check if an operation mode needs to be changed */
                result = ob1203_bio_mode_change_check(&ob1203_bio, &change);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_mode_change_check failed\r\n");
                }


                if (false != change)
                {
                    /* Stop the measurement */
                    result = ob1203_bio_measurement_stop(&ob1203_bio);
                    if (false == result)
                    {
                        APP_PRINT ("\r\nOB1203_bio_measurement_stop failed\r\n");
                    }

                    SensorOb1203data.spo2 = 0;
                    SensorOb1203data.heart_rate = 0;
                    SensorOb1203data.respiration_rate = 0;
                    SensorOb1203data.perfusion_index = 0;
                    /* Change to another mode */
                    measurementState = SENSOR_OB1203_INIT;
                }
                else
                {
                    /* Invalid data */
                    measurementState = SENSOR_OB1203_WAIT_MEASUREMENT;
                }
            }
        }
        break;

        case SENSOR_OB1203_AUTO_GAIN_CURRENT_CONTROL:
        {
            /* Auto gain and currents control */
            result = ob1203_bio_auto_gain_currents_control(&ob1203_bio,
                                                           &ppg_data,
                                                           &gain_currents,
                                                           &update);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_auto_gain_currents_control failed\r\n");
            }

            if (false != update)
            {
                /* Stop the measurement */
                result = ob1203_bio_measurement_stop(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_measurement_stop failed\r\n");
                }

                /* Reconfigure gain and currents */
                result = ob1203_bio_gain_currents_reconfigure(&ob1203_bio, &gain_currents);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_gain_currents_reconfigure failed\r\n");
                }

                measurementState = SENSOR_OB1203_START_MEASUREMENT;
            }
            else
            {
                measurementState = SENSOR_OB1203_CHECK_ALGO;
            }
        }
        break;

        case SENSOR_OB1203_CHECK_ALGO:
        {
            /* Check if the preparation for the algorithm is complete */
            result = ob1203_bio_algorithm_preparation_check(&ob1203_bio, &ready);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_algorithm_preparation_check failed\r\n");
            }

            if (false == ready)
            {
                /* Stop the measurement */
                result = ob1203_bio_measurement_stop(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_measurement_stop failed\r\n");
                }

                /* Reset the algorithm */
                result = ob1203_bio_algorithm_reset(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_algorithm_reset failed\r\n");
                }

                /* Clear PPG samples */
                result = ob1203_bio_samples_clear(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_samples_clear failed\r\n");
                }

                measurementState = SENSOR_OB1203_START_MEASUREMENT;
            }
            else
            {
                measurementState = SENSOR_OB1203_ADD_PPG_SAMPLES;
            }
        }
        break;

        case SENSOR_OB1203_ADD_PPG_SAMPLES:
        {
            /* Add PPG samples */
            result = ob1203_bio_samples_add(&ob1203_bio, &ppg_data);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_samples_add failed\r\n");
            }

            measurementState = SENSOR_OB1203_CALCULATE_HR_SP02;
        }
        break;

        case SENSOR_OB1203_CALCULATE_HR_SP02:
        {
            /* Calculate heart rate and SpO2 values */
            /* Disable interrupt to avoid context switch, in case sensor data is being copied by GetData function */
            portENTER_CRITICAL();
            result = ob1203_bio_hr_spo2_calculate(&ob1203_bio, (ob1203_bio_data_t *)&SensorOb1203data);
            portEXIT_CRITICAL();
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_hr_spo2_calculate failed\r\n");
            }
            else
            {
                if ((SensorOb1203data.heart_rate != 0) || (SensorOb1203data.spo2 != 0))
                {
                    print_ob_data();
                }
            }

            measurementState = SENSOR_OB1203_CALCULATE_RESPIRATION_RATE;
        }
        break;

        case SENSOR_OB1203_CALCULATE_RESPIRATION_RATE:
        {
            /* Calculate a respiration rate value */
            /* Disable interrupt to avoid context switch, in case sensor data is being copied by GetData function */
            portENTER_CRITICAL();
            result = ob1203_bio_rr_calculate(&ob1203_bio,
                                             (ob1203_bio_data_t *)&SensorOb1203data);
            portEXIT_CRITICAL();
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_rr_calculate failed\r\n");
            }
            measurementState = SENSOR_OB1203_CHECK_PERFUSION_INDEX;
        }
        break;

        case SENSOR_OB1203_CHECK_PERFUSION_INDEX:
        {
            /* Check perfusion index (PI) */
            result = ob1203_bio_perfusion_index_check(&ob1203_bio,
                                                      &valid);
            if (false == result)
            {
                APP_PRINT ("\r\nOB1203_bio_perfusion_index_check failed \r\n");
            }

            if (false != valid)
            {
                measurementState = SENSOR_OB1203_WAIT_MEASUREMENT;
            }
            else
            {
                /* Stop the measurement */
                APP_PRINT ("\r\n **Bio Perfusion Index : False \n");
                result = ob1203_bio_measurement_stop(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_measurement_stop failed\r\n");
                }

                /* Reset the algorithm */
                result = ob1203_bio_algorithm_reset(&ob1203_bio);
                if (false == result)
                {
                    APP_PRINT ("\r\nOB1203_bio_algorithm_reset failed\r\n");
                }

                measurementState = SENSOR_OB1203_START_MEASUREMENT;
            }
        }
        break;

        default:
        {
            APP_PRINT ("\r\nOB1203_bio DEFAULT\r\n");
        }
        break;
    }
}

void Sensor_Ob1203GetData(ob1203_bio_data_t *data)
{
    *data = SensorOb1203data;
}
