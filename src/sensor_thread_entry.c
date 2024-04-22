/***********************************************************************************************************************
 * File Name    : sensor_thread_entry.c
 * Description  : Contains data structures and functions
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
#include "sensor_thread.h"
#include <console.h>
#include <sensor_config.h>
#include <sensor_iaq.h>
#include <sensor_oaq.h>
#include <sensor_hs3001.h>
#include <sensor_icp10101.h>
#include <sensor_icm20948.h>

#define UNUSED(x)  ((void)(x))
#define INT_CHANNEL (1)
/* Define macros for I2C pins */
/* I2C channel 0 */
#define I2C_SDA_0 (BSP_IO_PORT_04_PIN_01)
#define I2C_SCL_0 (BSP_IO_PORT_04_PIN_00)


extern TaskHandle_t sensor_thread;
extern TaskHandle_t oximeter_thread;
extern TaskHandle_t zmod_thread;
extern TaskHandle_t console_thread;



/*******************************************************************************************************************//**
 * @brief       Re-cover the I2C bus if it is busy
 * @param[in]   SCL SCL pin of the IIC bus
 * @param[in]   SDA SDA pin of the IIC bus
 * @retval      None
 ***********************************************************************************************************************/
void Sensor_RecoverI2cBus(const bsp_io_port_pin_t SCL, const bsp_io_port_pin_t SDA)
{
    const bsp_io_port_pin_t PIN_SCL = SCL;
    const bsp_io_port_pin_t PIN_SDA = SDA;
    const uint32_t pinToggleDelay = 5;

    /* switch to GPIO mode, N-channel open-drain with pull-up */
    R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_SCL, (uint32_t)IOPORT_CFG_NMOS_ENABLE | (uint32_t)IOPORT_CFG_PORT_DIRECTION_OUTPUT | (uint32_t)IOPORT_CFG_PORT_OUTPUT_HIGH | (uint32_t)IOPORT_CFG_PULLUP_ENABLE);
    R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_SDA, (uint32_t)IOPORT_CFG_NMOS_ENABLE | (uint32_t)IOPORT_CFG_PORT_DIRECTION_OUTPUT | (uint32_t)IOPORT_CFG_PORT_OUTPUT_HIGH | (uint32_t)IOPORT_CFG_PULLUP_ENABLE);

    /* toggle SCL 9+ times */
    for (uint32_t i = 0; i< 10; i++) {
        R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SCL, BSP_IO_LEVEL_LOW);
        R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);
        R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SCL, BSP_IO_LEVEL_HIGH);
        R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);
    }

    /* generate STOP condition (SDA going from LOW to HIGH when SCL is HIGH) */
    R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SCL, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);
    R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SDA, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);
    R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SCL, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);
    R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_SDA, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(pinToggleDelay, BSP_DELAY_UNITS_MICROSECONDS);

    /* switch back to peripheral mode */
    R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_SCL, (uint32_t)IOPORT_CFG_PERIPHERAL_PIN | (uint32_t)IOPORT_PERIPHERAL_IIC);
    R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_SDA, (uint32_t)IOPORT_CFG_PERIPHERAL_PIN | (uint32_t)IOPORT_PERIPHERAL_IIC);
}

/*******************************************************************************************************************//**
 * @brief       Quick setup for g_comms_i2c_bus0
 * @param[in]   p_args
 * @retval
 * @retval
 ***********************************************************************************************************************/
void Sensor_I2cBus0Init(void)
{
    fsp_err_t err;
    i2c_master_instance_t *p_driver_instance = (i2c_master_instance_t*) g_comms_i2c_bus0_extended_cfg.p_driver_instance;
    /* Open I2C driver, this must be done before calling any COMMS API */
    err = p_driver_instance->p_api->open (p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_DBG_PRINT("\r\nI2C bus setup unsuccess\r\n");
        APP_ERR_TRAP(err);
    }
    else
    {
        APP_DBG_PRINT("\r\nI2C bus setup success\r\n");
    }

    /* Recover if I2C0 bus is busy */
    Sensor_RecoverI2cBus(I2C_SCL_0, I2C_SDA_0);

    /* Create a semaphore for blocking if a semaphore is not NULL */
    if (NULL != g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore)
    {
        *(g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_handle) = xSemaphoreCreateCountingStatic (
                (UBaseType_t) 1, (UBaseType_t) 0,
                g_comms_i2c_bus0_extended_cfg.p_blocking_semaphore->p_semaphore_memory);
    }

    /* Create a recursive mutex for bus lock if a recursive mutex is not NULL */
    if (NULL != g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex)
    {
        *(g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_handle) = xSemaphoreCreateRecursiveMutexStatic (
                g_comms_i2c_bus0_extended_cfg.p_bus_recursive_mutex->p_mutex_memory);
    }
}

/*******************************************************************************************************************//**
 * @brief       Reset zmod sensor
 * @param[in]   None
 * @retval      None
 ***********************************************************************************************************************/
static void Sensor_ResetZmod(void)
{
    R_BSP_PinAccessEnable ();

    /* ZMOD Reset for CK-RA5M5 */
    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_06, BSP_IO_LEVEL_HIGH); // 4510
    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH); // 4410
    vTaskDelay (10);

    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_06, BSP_IO_LEVEL_LOW); // 4510
    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_LOW); // 4410
    vTaskDelay (10);

    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_06, BSP_IO_LEVEL_HIGH); // 4510
    R_BSP_PinWrite ((bsp_io_port_pin_t) BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH); // 4410
    vTaskDelay (10);

    R_BSP_PinAccessDisable ();
}

/*******************************************************************************************************************//**
 * @brief       sensor_thread_entry function
 * @param[in]   pvParameters
 * @retval      None
 ***********************************************************************************************************************/
void sensor_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    Sensor_I2cBus0Init();
    Sensor_ResetZmod();

    /* Start Oximeter thread execution */
    xTaskNotifyFromISR(oximeter_thread, 1, 1, NULL);

#if  _HS3001_SENSOR_ENABLE_
    /* Open HS3001 */
    SensorHs3001_Init();
#endif

#if  _ZMOD4410_SENSOR_ENABLE_
    /* Open IAQ ZMOD4410 */
    SensorIaq_Init();
#endif

#if _ZMOD4510_SENSOR_ENABLE_
    /* Open ZMOD4510 */
    SensorOaq_Init();
#endif

#if	_ICP10101_SENSOR_ENABLE_
    /* Initialize ICP10101 sensor */
    SensorIcp10101_Init();
#endif

#if _ICM20948_SENSOR_ENABLE_
    /* Open ICM20948 */
    SensorIcm20948_Init ();
#endif

	xTaskNotifyFromISR(console_thread, 1, 1, NULL);

    /* wait for application thread to finish MQTT connection */
    xTaskNotifyWait(pdFALSE, pdFALSE, (uint32_t* )&sensor_thread, portMAX_DELAY);

    /* Start thread for IAQ ZMOD 4410 data acquisition */
    xTaskNotifyFromISR(zmod_thread, 1, 1, NULL);

    while (1)
    {

#if _HS3001_SENSOR_ENABLE_
        /* Read HS3001 sensor data */
        SensorHs3001_MainFunction();
#endif

#if  _ICP10101_SENSOR_ENABLE_
        /* Read ICP10101 sensor data and send it to the queue */
        SensorIcp10101_MainFunction();
#endif

#if _ZMOD4510_SENSOR_ENABLE_
        /* Read ZMOD4510 sensor data */
        SensorOaq_MainFunction();
#endif

#if _ICM20948_SENSOR_ENABLE_
        /* Read ICM20948 sensor data and send it to the queue */
        SensorIcm20948_MainFunction();
#endif
        vTaskDelay (5);

    }
}
