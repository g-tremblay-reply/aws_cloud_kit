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
    MQTTContext_t CloudAppMqtt;
    MQTTStatus_t mqttStatus = MQTTServerRefused;

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Wait for the cloud_app_thread to be notified before starting. This notification typically
     * come from Console thread that takes user input before starting cloud app */
    xTaskNotifyWait(pdFALSE, pdFALSE, NULL, portMAX_DELAY);

    /* Try to initialize CloudApp with stored credentials from CLoudProv */
    mqttStatus = CloudProv_Init(&CloudAppMqtt, CloudApp_MqttCallback);

    if(mqttStatus == MQTTIllegalState)
    {
        /* MQTT endpoint is unreachable, thus don't try further to provision device */
        while(1)
        {
            APP_WARN_PRINT("MQTT Broker endpoint is not reachable"
                           "\r\nPlease reset Cloud Kit [ORANGE]while spamming SPACE BAR[YELLOW] "
                           "to open MENU and try new MQTT Broker endpoint\r\n\r\n");
            vTaskDelay(10000);
        }
    }
    else if(mqttStatus != MQTTSuccess)
    {
        /* Connection to MQTT was unsuccessful. This might be caused by the certificate chain being invalid,
         * Thus, try to provision device via fleet provisioning */
        mqttStatus = CloudProv_ProvisionDevice(&CloudAppMqtt, CloudApp_MqttCallback);
        if(mqttStatus != MQTTSuccess)
        {
            /* MQTT endpoint is unreachable, thus don't try further to provision device */

            APP_WARN_PRINT("CloudApp could not authenticate with given credentials."
                           "\r\nPlease reset Cloud Kit [ORANGE]while spamming SPACE BAR[YELLOW] "
                           "to open MENU and try new credentials\r\n\r\n");
        }
    }

    if(mqttStatus == MQTTSuccess)
    {
        mqttStatus = CloudApp_SubscribeTopics(&CloudAppMqtt);
    }

    if(mqttStatus == MQTTSuccess)
    {
        APP_PRINT(("Device is ready for Receiving/Publishing messages from AWS Iot \r\n\r\n"));
    }
    else
    {
        APP_WARN_PRINT(("Device is not connected to AWS IoT server, but will still print sensor reading" \
        " on Console. \r\n\r\n"));
    }

    xTaskNotifyFromISR(sensor_thread, 1, 1, NULL);

    /* Start updating cyclic request to publish sensor data
     * Those requests are processed in CloudApp_MainFunction() */
    CloudApp_EnableDataPushTimer();

    while (1)
    {
        CloudApp_MainFunction(&CloudAppMqtt);
    }
}
