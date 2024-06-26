/***********************************************************************************************************************
 * File Name    : app_thread_entry.c
 * Description  : Contains data structures and functions used in Cloud Connectivity application
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
#include <cloud_app_thread.h>
#include <console.h>
#include <cloud_prov.h>
#include <cloud_app.h>

/*************************************************************************************
 * Macro definitions
 ************************************************************************************/
extern TaskHandle_t sensor_thread;
extern TaskHandle_t console_thread;


/*************************************************************************************
 * global functions
 ************************************************************************************/


/*************************************************************************************
 * global variables
 ************************************************************************************/

/*************************************************************************************
 * Private variables
 ************************************************************************************/

/*******************************************************************************************************************//**
 * @brief      Application Thread entry function
 * @param[in]   pvParameters     contains TaskHandle_t
 * @retval
 * @retval
 ***********************************************************************************************************************/
void cloud_app_thread_entry(void *pvParameters)
{
    MQTTContext_t CloudAppMqtt = {0u};
    MQTTStatus_t mqttStatus = MQTTServerRefused;

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Wait for the cloud_app_thread to be notified before starting. This notification typically
     * come from Console thread that takes user input before starting cloud app */
    xTaskNotifyWait(pdFALSE, pdFALSE, NULL, portMAX_DELAY);

    /* Try to connect to MQTT and provision device if needed */
    mqttStatus = CloudProv_Init(&CloudAppMqtt, CloudApp_MqttCallback);

    if(mqttStatus == MQTTSuccess)
    {
         CloudApp_Init(&CloudAppMqtt);
    }

    xTaskNotifyFromISR(sensor_thread, 1, 1, NULL);

    while (1)
    {
        CloudApp_MainFunction(&CloudAppMqtt);
    }
}
