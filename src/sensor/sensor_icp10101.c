/***********************************************************************************************************************
 * File Name    : sensor_icp10101.c
 * Description  : Contains data structures and functions for icp10101 sensor
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
#include <console.h>
#include <sensor_icp10101.h>
#include <sensor_thread.h>

#define I2C_TRANSMISSION_IN_PROGRESS        (0)
#define I2C_TRANSMISSION_COMPLETE           (1)
#define I2C_TRANSMISSION_ABORTED            (2)


#define ICP_I2C_ID 0x63

#define ICP_CMD_READ_ID   (uint16_t) 0xefc8
#define ICP_CMD_SET_ADDR  0xc595
#define ICP_CMD_READ_OTP  0xc7f7
#define ICP_CMD_MEAS_LP   0x609c
#define ICP_CMD_MEAS_N    0x6825
#define ICP_CMD_MEAS_LN   0x70df
#define ICP_CMD_MEAS_ULN  0x7866

// constants for presure calculation
const float _pcal[3] = { 45000.0, 80000.0, 105000.0 };
const float _lut_lower = 3.5 * 0x100000;    // 1<<20
const float _lut_upper = 11.5 * 0x100000;   // 1<<20
const float _quadr_factor = 1 / 16777216.0;
const float _offst_factor = 2048.0;

#define I2C_TRANSMISSION_IN_PROGRESS        0
#define I2C_TRANSMISSION_COMPLETE           1
#define I2C_TRANSMISSION_ABORTED            2

typedef enum
{
    SENSOR_ICP10101_START_MEASUREMENT,
    SENSOR_ICP10101_WAIT_END_OF_MEASUREMENT,
    SENSOR_ICP10101_READ_RESULT,
} SensorIcp10101_State_t;

typedef enum
{
    SENSOR_ICP10101_FAST,
    SENSOR_ICP10101_NORMAL,
    SENSOR_ICP10101_ACCURATE,
    SENSOR_ICP10101_VERY_ACCURATE
}SensorIcp10101_MeasureMode_t;


float SensorIcpCalibration[4];
static float_t SensorIcpTemperatureC;
static float_t SensorIcpPressurePa;
static uint8_t SensorIcpMeasurementDuration;
static uint32_t SensorIcpMeasurementTimer;
static uint8_t ICP_transmit_complete_flag;
static SensorIcp10101_State_t SensorIcpState;

static fsp_err_t SensorIcp10101_Read(uint8_t *res_buf, uint8_t res_len);


static fsp_err_t SensorIcp10101_Write(uint8_t *cmd, uint32_t byteCount)
{
    uint16_t timeout = 1000;
    int8_t err;
    //TODO test that byte order is correct when *cmd given is of uint16
    ICP_transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    err = g_comms_i2c_device4.p_api->write (g_comms_i2c_device4.p_ctrl, cmd, (uint32_t) (byteCount));
    if (err == FSP_SUCCESS)
    {
        while (ICP_transmit_complete_flag == I2C_TRANSMISSION_IN_PROGRESS)
        {
            if (--timeout == 0)
            {
                break;
            }
            vTaskDelay (1);
        }

        ICP_transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    }

    if (ICP_transmit_complete_flag == I2C_TRANSMISSION_ABORTED)
    {
        err = FSP_ERR_ABORTED;
    }

    return err;
}

/**************************************************************************************
 * @brief   Read Response over I2C
 * @param[in]res_buf pointer to Response Buffer
 * @param[in]res_len Length
 * @param[in]
 * @retval
 **************************************************************************************/
static fsp_err_t SensorIcp10101_Read(uint8_t *res_buf, uint8_t res_len)
{
    uint16_t timeout = 1000;
    fsp_err_t err;
    ICP_transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    err = g_comms_i2c_device4.p_api->read (g_comms_i2c_device4.p_ctrl, res_buf, res_len);
    if (err == FSP_SUCCESS)
    {
        while (ICP_transmit_complete_flag == I2C_TRANSMISSION_IN_PROGRESS)
        {
            if (--timeout == 0)
            {
                break;
            }
            vTaskDelay (1);
        }

        ICP_transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    }

    if (ICP_transmit_complete_flag == I2C_TRANSMISSION_ABORTED)
    {
        err = FSP_ERR_ABORTED;
    }
    return err;
}


/**************************************************************************************
 * @brief  isConnected
 * @param[in]
 * @param[in]
 * @param[in]
 * @retval  True if  sensor is connected , False otherwise
 **************************************************************************************/

static bool isConnected(void){
    uint8_t id_buf[2];
    uint16_t id;
    uint16_t cmd = ICP_CMD_READ_ID;
    uint8_t cmdBuff[2]={0};
    cmdBuff[0] = (uint8_t)((cmd >> 8) & 0xFF);
    cmdBuff[1] = (uint8_t)(cmd & 0xFF);

    SensorIcp10101_Write((uint8_t *) cmdBuff, 2u);
    SensorIcp10101_Read(id_buf, 2);
    id = (uint16_t)(((uint16_t)(id_buf[0u]) << 8) | id_buf[1u]);
    if ((id & 0x3F) == 0x8)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static fsp_err_t SensorIcp10101_StartMeasurement(SensorIcp10101_MeasureMode_t mode){
	uint16_t cmd;
    fsp_err_t err;

	switch (mode)
    {
        case SENSOR_ICP10101_FAST:
            cmd = ICP_CMD_MEAS_LP;
            SensorIcpMeasurementDuration = 3;
            break;
        case SENSOR_ICP10101_ACCURATE:
            cmd = ICP_CMD_MEAS_LN;
            SensorIcpMeasurementDuration = 24;
            break;
        case SENSOR_ICP10101_VERY_ACCURATE:
            cmd = ICP_CMD_MEAS_ULN;
            SensorIcpMeasurementDuration = 95;
            break;
        case SENSOR_ICP10101_NORMAL:
        default:
            cmd = ICP_CMD_MEAS_N;
            SensorIcpMeasurementDuration = 7;
            break;
	}

    uint8_t cmdBuff[2]={0};
    cmdBuff[0] = (uint8_t)((cmd >> 8) & 0xFF);
    cmdBuff[1] = (uint8_t)(cmd & 0xFF);
    err = SensorIcp10101_Write((uint8_t *) &cmdBuff, 2u);

    if(err == FSP_SUCCESS)
    {
        /* Reset measurement timer */
        SensorIcpMeasurementTimer = 0u;
    }

    return err;
}

static float_t SensorIcp10101_CalculateTempC(uint16_t rawTempData)
{
    float_t temperature;
    temperature = -45.f + 175.f / 65536.f * (float_t)rawTempData;
    return  temperature;
}



static float_t SensorIcp10101_CalculatePressure(uint32_t rawPressureData)
{
    float_t pressure;
	float_t t = (float_t)(rawPressureData - 32768);
	float_t s1 = _lut_lower + (float_t)(SensorIcpCalibration[0] * t * t) * _quadr_factor;
	float_t s2 = _offst_factor * SensorIcpCalibration[3] + (float_t)(SensorIcpCalibration[1] * t * t) * _quadr_factor;
	float_t s3 = _lut_upper + (float_t)(SensorIcpCalibration[2] * t * t) * _quadr_factor;
	float_t c = (s1 * s2 * (_pcal[0] - _pcal[1]) +
			s2 * s3 * (_pcal[1] - _pcal[2]) +
			s3 * s1 * (_pcal[2] - _pcal[0])) /
					(s3 * (_pcal[0] - _pcal[1]) +
							s1 * (_pcal[1] - _pcal[2]) +
							s2 * (_pcal[2] - _pcal[0]));
	float_t a = (_pcal[0] * s1 - _pcal[1] * s2 - (_pcal[1] - _pcal[0]) * c) / (s1 - s2);
	float_t b = (_pcal[0] - a) * (s1 + c);
    pressure = (float_t)((a + b / (c + (float_t)rawPressureData)));
    return pressure;
}

static void SensorIcp10101_ReadCalibration(void)
{
    // read sensor calibration data
    uint8_t addr_otp_cmd[5] = {
            (ICP_CMD_SET_ADDR >> 8) & 0xff,
            ICP_CMD_SET_ADDR & 0xff,
            0x00, 0x66, 0x9c };
    uint8_t otp_buf[3];
    uint16_t readCmd = ICP_CMD_READ_OTP;
    uint8_t cmdBuff[2]={0};

    cmdBuff[0] = (uint8_t)((readCmd >> 8) & 0xFF);
    cmdBuff[1] = (uint8_t)(readCmd & 0xFF);
    SensorIcp10101_Write(addr_otp_cmd, 5);
    for (int i=0; i<4; i++) {
        SensorIcp10101_Write((uint8_t *) &cmdBuff, 2u);
        SensorIcp10101_Read(otp_buf, 3u);
        SensorIcpCalibration[i] = (float) ((otp_buf[0] << 8) | otp_buf[1]);
    }
}



void SensorIcp10101_TimerCallback(timer_callback_args_t *p_args)
{
	if ((p_args->event == TIMER_EVENT_CYCLE_END) &&
        (SensorIcpState == SENSOR_ICP10101_WAIT_END_OF_MEASUREMENT))
    {
		SensorIcpMeasurementTimer++;
        if(SensorIcpMeasurementTimer >= SensorIcpMeasurementDuration)
        {
            SensorIcpState = SENSOR_ICP10101_READ_RESULT;
        }
	}
}


void SensorIcp10101_CommCallback(rm_comms_callback_args_t *p_args)
{
    if (p_args->event == RM_COMMS_EVENT_OPERATION_COMPLETE)
    {
        ICP_transmit_complete_flag = I2C_TRANSMISSION_COMPLETE;
    }
    else
    {
        ICP_transmit_complete_flag = I2C_TRANSMISSION_ABORTED;
    }
}

void SensorIcp10101_Init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Open i2c device for Icp10101 sensor */
    err = g_comms_i2c_device4.p_api->open (g_comms_i2c_device4.p_ctrl, g_comms_i2c_device4.p_cfg);
    if (FSP_SUCCESS == err)
    {
        APP_PRINT("\r\nICP10101 sensor setup success\r\n");
    }
    else
    {
        APP_DBG_PRINT("\r\nICP10101 open sensor instance failed: %d\n", err);
    }

    /* Start timer that allows processing of measurement time in
     * SensorIcp10101_TimerCallback function */
    g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg);
    g_timer2.p_api->enable(g_timer2.p_ctrl);
    g_timer2.p_api->start(g_timer2.p_ctrl);

    // verify that the sensor is responding
    if (isConnected())
    {
        SensorIcp10101_ReadCalibration();
        SensorIcpState = SENSOR_ICP10101_START_MEASUREMENT;
    }
    else
    {
        APP_ERR_PRINT("\r\n Cannot communicate with ICP10101 sensor. All hell breaks loose %d\n");
    }
}

void SensorIcp10101_MainFunction(void)
{
    switch(SensorIcpState)
    {
        case SENSOR_ICP10101_START_MEASUREMENT:
            SensorIcp10101_StartMeasurement(SENSOR_ICP10101_VERY_ACCURATE);
            SensorIcpState = SENSOR_ICP10101_WAIT_END_OF_MEASUREMENT;
            break;
        case SENSOR_ICP10101_WAIT_END_OF_MEASUREMENT:
            break;
        case SENSOR_ICP10101_READ_RESULT:
        {
            uint16_t rawTemp;
            uint32_t rawPressure;
            uint8_t res_buf[9];

            /* Read result from sensor */
            SensorIcp10101_Read(res_buf, 9);

            /* extract raw temperature */
            rawTemp = (uint16_t)(((res_buf[0]) << 8) | res_buf[1]);
            uint32_t L_res_buf3 = res_buf[3];   // expand result bytes to 32bit to fix issues on 8-bit MCUs
            uint32_t L_res_buf4 = res_buf[4];
            uint32_t L_res_buf6 = res_buf[6];

            /* extract raw pressure */
            rawPressure = (L_res_buf3 << 16) | (L_res_buf4 << 8) | L_res_buf6;

            SensorIcpPressurePa = SensorIcp10101_CalculatePressure(rawPressure);
            SensorIcpTemperatureC = SensorIcp10101_CalculateTempC(rawTemp);
            SensorIcpState = SENSOR_ICP10101_START_MEASUREMENT;
            break;
        }
        default:
            break;
    }
}

void SensorIcp10101_GetData(float_t *temperature, float_t *pressure)
{
    *temperature = SensorIcpTemperatureC;
    *pressure = SensorIcpPressurePa;
}