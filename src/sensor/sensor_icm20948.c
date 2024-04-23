/***********************************************************************************************************************
 * File Name    : sensor_icm20948.c
 * Description  : Contains data structures and functions used in Console related application
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
#include "sensor_icm20948.h"
#include <sensor_thread.h>



#define I2C_TRANSMISSION_IN_PROGRESS        (0)
#define I2C_TRANSMISSION_COMPLETE           (1)
#define I2C_TRANSMISSION_ABORTED            (2)
#define sq(x)  (x*x)
#define M_PI        3.14159265358979323846

char RawAcc_x[20],RawAcc_y[20],RawAcc_z[20];
char CorrAcc_x[20],CorrAcc_y[20],CorrAcc_z[20];
char Gval_x[20],Gval_y[20],Gval_z[20];
char RawGyro_x[20],RawGyro_y[20],RawGyro_z[20];
char CorGyro_x[20],CorGyro_y[20],CorGyro_z[20];
char Mag_x[20],Mag_y[20],Mag_z[20];
char ResultG[20];

float resultantG;

int i2cAddress;

static uint8_t currentBank;
static uint8_t AccelResultsBuffer[20];
static uint8_t accRangeFactor = 1u;
static uint8_t gyrRangeFactor = 1u;
static uint8_t regVal;   // intermediate storage of register values

static xyzFloat corrAccRaw;
static xyzFloat accRaw;
static xyzFloat gVal;
static xyzFloat gyrRaw;
static xyzFloat gyr;
static xyzFloat magValue;
static xyzFloat accOffsetVal = {0};
static xyzFloat accCorrFactor = {1.0,
                            1.0,
                            1.0};
static xyzFloat gyrOffsetVal = {0};

static ICM20948_fifoType fifoType;
static uint8_t transmit_complete_flag;

/* Basic settings */
static void configIcmOffsets();
static void setGyrOffsets(float xOffset, float yOffset, float zOffset);
static void setAccRange(ICM20948_accRange accRange);
static void setAccDLPF(ICM20948_dlpf dlpf);
static void setAccSampleRateDivider(uint16_t accSplRateDiv);
static void setGyrRange(ICM20948_gyroRange gyroRange);
static void setGyrDLPF(ICM20948_dlpf dlpf);
static void setGyrSampleRateDivider(uint8_t gyrSplRateDiv);
static void setTempDLPF(ICM20948_dlpf dlpf);
static void setI2CMstSampleRate(uint8_t rateExp);
static void setIcmSleepMode(bool sleep);


/* x,y,z results */
static void getAccRawValues(xyzFloat*a);
static void getCorrectedAccRawValues(xyzFloat*a);
static void getGValues(xyzFloat*a);
static void getCorrectedAccRawValuesFromFifo(xyzFloat*a);
static double getResultantG(xyzFloat gVal);
static float getTemperature();
static void getGyrRawValues(xyzFloat*a);
static void getCorrectedGyrRawValues(xyzFloat*a);
static void getGyrValues(xyzFloat*a);
static xyzFloat getGyrValuesFromFifo();
static void getMagValues(xyzFloat*a);
static void readAccelResults(uint8_t *data);

/* Angles and Orientation */
static void getAngles(xyzFloat*a);
static ICM20948_orientation getOrientation();
static float getPitch();
static float getRoll();

/* Power, Sleep, Standby */
static void enableCycle(ICM20948_cycle cycle);
static void enableLowPower(bool enLP);
static void setGyrAverageInCycleMode(ICM20948_gyroAvgLowPower avg);
static void setAccAverageInCycleMode(ICM20948_accAvgLowPower avg);
static uint8_t requestIcmReset();

/* Interrupts */
static void setIntPinPolarity(ICM20948_intPinPol pol);
static void enableIntLatch(bool latch);
static void enableClearIntByAnyRead(bool clearByAnyRead);
static void setFSyncIntPolarity(ICM20948_intPinPol pol);
static void enableInterrupt(ICM20948_intType intType);
static void disableInterrupt(ICM20948_intType intType);
static uint8_t readAndClearInterrupts();
static bool checkInterrupt(uint8_t source, ICM20948_intType type);
static void setWakeOnMotionThreshold(uint8_t womThresh, ICM20948_womCompEn womCompEn);

/* FIFO */
static void enableFifo(bool fifo);
static void setFifoMode(ICM20948_fifoMode mode);
static void startFifo(ICM20948_fifoType fifo);
static void stopFifo();
static void resetFifo();
static int16_t getFifoCount();
static int16_t getNumberOfFifoDataSets();
static void findFifoBegin();

/* Magnetometer */
static void enableMagDataRead(uint8_t reg, uint8_t bytes);
static void initSlaveMagnetometer(void);
static int16_t whoAmIMag();
static void setMagOpMode(AK09916_opMode opMode);
static void resetMag();

void setClockToAutoSelect();
void correctAccRawValues(xyzFloat *a);
void correctGyrRawValues(xyzFloat *a);
void switchBank(uint8_t newBank);
fsp_err_t writeRegister8(uint8_t bank, uint8_t reg, uint8_t val);
fsp_err_t writeRegister16(uint8_t bank, uint8_t reg, uint16_t val);
uint8_t readRegister8(uint8_t bank, uint8_t reg);
int16_t readRegister16(uint8_t bank, uint8_t reg);
static int16_t readAK09916Register16(uint8_t reg);
static uint8_t readAK09916Register8(uint8_t reg);
static void writeAK09916Register8(uint8_t reg, uint8_t val);
void readICM20948xyzValFromFifo(xyzFloat*a);
static fsp_err_t writeDevice(uint8_t reg, uint8_t *val, uint8_t num);
static fsp_err_t readDevice(uint8_t reg, uint8_t *val, uint8_t num);


fsp_err_t writeDevice(uint8_t reg, uint8_t *val, uint8_t num)
{
    uint16_t timeout = 1000;
    fsp_err_t err;
    static uint8_t data[50];
    data[0] = reg;
    memcpy (&(data[1]), val, (size_t) num);
    transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    err = g_comms_i2c_device5.p_api->write (g_comms_i2c_device5.p_ctrl, data, (uint32_t) (num + 1));
    if (err == FSP_SUCCESS)
    {
        while (transmit_complete_flag == I2C_TRANSMISSION_IN_PROGRESS)
        {
            if (--timeout == 0)
            {
                break;
            }
            vTaskDelay (1);
        }

        transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    }

    if (transmit_complete_flag == I2C_TRANSMISSION_ABORTED)
    {
        err = FSP_ERR_ABORTED;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief       RMComm I2C read
 * @param[in]   register address, buffer pointer to store read data and number of to read
 * @retval      FSP_SUCCESS         Upon successful open and start of timer
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 ***********************************************************************************************************************/
fsp_err_t readDevice(uint8_t reg, uint8_t *val, uint8_t num)
{
    uint16_t timeout = 1000;
    fsp_err_t err;
    transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    rm_comms_write_read_params_t write_read_params;
    write_read_params.p_src = &reg;
    write_read_params.src_bytes = 1;
    write_read_params.p_dest = val;
    write_read_params.dest_bytes = (uint8_t) num;
    err = g_comms_i2c_device5.p_api->writeRead (g_comms_i2c_device5.p_ctrl, write_read_params);
    if (err == FSP_SUCCESS)
    {
        while (transmit_complete_flag == I2C_TRANSMISSION_IN_PROGRESS)
        {
            if (--timeout == 0)
            {
                break;
            }
            vTaskDelay (1);
        }

        transmit_complete_flag = I2C_TRANSMISSION_IN_PROGRESS;
    }

    if (transmit_complete_flag == I2C_TRANSMISSION_ABORTED)
    {
        err = FSP_ERR_ABORTED;
    }
    return err;
}


/**************************************************************************************
 * @brief     Set offset values for ICM_20948 Motion Tracking sensor
 * @param[in]
 * @retval
 **************************************************************************************/

void configIcmOffsets(void)
{
    xyzFloat accRawVal, gyrRawVal;
    accOffsetVal.x = 0.0;
    accOffsetVal.y = 0.0;
    accOffsetVal.z = 0.0;

    /* Set to lowest noise */
    setGyrDLPF (ICM20948_DLPF_6);
    /* Set to highest resolution */
    setGyrRange (ICM20948_GYRO_RANGE_250);
    setAccRange (ICM20948_ACC_RANGE_2G);
    setAccDLPF (ICM20948_DLPF_6);
    vTaskDelay (50);

    for (int i = 0; i < 50; i++)
    {
        readAccelResults(AccelResultsBuffer);
        getAccRawValues (&accRawVal);
        accOffsetVal.x += accRawVal.x;
        accOffsetVal.y += accRawVal.y;
        accOffsetVal.z += accRawVal.z;
        vTaskDelay (10);
    }

    accOffsetVal.x /= 50;
    accOffsetVal.y /= 50;
    accOffsetVal.z /= 50;
    accOffsetVal.z -= 16384.0;

    for (int i = 0; i < 50; i++)
    {
        readAccelResults(AccelResultsBuffer);
        getGyrRawValues (&gyrRawVal);

        gyrOffsetVal.x += gyrRawVal.x;
        gyrOffsetVal.y += gyrRawVal.y;
        gyrOffsetVal.z += gyrRawVal.z;
        R_BSP_SoftwareDelay (1, BSP_DELAY_UNITS_MILLISECONDS);
    }

    gyrOffsetVal.x /= 50;
    gyrOffsetVal.y /= 50;
    gyrOffsetVal.z /= 50;
}

/**************************************************************************************
 * @brief     Set Gyrometer offset ICM_20948 Motion Tracking sensor
 * @param[in] xOffset  yOffset zOffset : x,y,z axis offset value
 * @retval
 **************************************************************************************/

void setGyrOffsets(float xOffset, float yOffset, float zOffset)
{
    gyrOffsetVal.x = xOffset;
    gyrOffsetVal.y = yOffset;
    gyrOffsetVal.z = zOffset;
}

/* Sub Functions */


/**************************************************************************************
 * @brief     Set Accelerometer range ICM_20948 Motion Tracking sensor
 * @param[in] accRange Gravitational force limit enumeration
 * @retval
 **************************************************************************************/

void setAccRange(ICM20948_accRange accRange)
{
    regVal = readRegister8 (2, ICM20948_ACCEL_CONFIG);
    regVal = (uint8_t) (regVal & ~(ICM20948_PWR_MGMT_1 ));
    regVal = (uint8_t) (regVal | (accRange << 1u ));
    writeRegister8 (2u, ICM20948_ACCEL_CONFIG, regVal);
    accRangeFactor = (uint8_t) (1u << accRange);
}

/**************************************************************************************
 * @brief     Set Accelerometer Low Pass Filter frequency
 * @param[in] dlpf Low Pass Filter frequency
 * @retval
 **************************************************************************************/

void setAccDLPF(ICM20948_dlpf dlpf)
{
    regVal = readRegister8 (2, ICM20948_ACCEL_CONFIG);
    if (dlpf == ICM20948_DLPF_OFF)
    {
        regVal &= 0xFE;
        writeRegister8 (2, ICM20948_ACCEL_CONFIG, regVal);
        return;
    }
    else
    {
        regVal |= 0x01;
        regVal &= 0xC7;
        regVal = (uint8_t) (regVal | (dlpf << 3));
    }
    writeRegister8 (Bank_2, ICM20948_ACCEL_CONFIG, regVal);
}

/**************************************************************************************
 * @brief     Set Accelerometer Sample Rate Divider
 * @param[in] accSplRateDiv  Sample Rate Divider value
 * @retval
 **************************************************************************************/

void setAccSampleRateDivider(uint16_t accSplRateDiv)
{
    writeRegister8 (Bank_2, ICM20948_ACCEL_SMPLRT_DIV_1, (uint8_t) accSplRateDiv);
}

/**************************************************************************************
 * @brief     Set Gyroscope range
 * @param[in] gyroRange  Gyroscope range enumeration
 * @retval
 **************************************************************************************/

void setGyrRange(ICM20948_gyroRange gyroRange)
{
    regVal = readRegister8 (Bank_2, ICM20948_GYRO_CONFIG_1);
    regVal = (uint8_t) (regVal & (~(ICM20948_YG_OFFS_USRL )));
    regVal |= (uint8_t) (gyroRange << (uint8_t) 1);
    writeRegister8 (Bank_2, ICM20948_GYRO_CONFIG_1, regVal);
    gyrRangeFactor = (uint8_t) (1u << gyroRange);
}

/**************************************************************************************
 * @brief     Set Gyroscope Low Pass Filter Frequency
 * @param[in] dlpf  Low Pass Filter Frequency enumeration
 * @retval
 **************************************************************************************/
void setGyrDLPF(ICM20948_dlpf dlpf)
{
    regVal = readRegister8 (Bank_2, ICM20948_GYRO_CONFIG_1);
    if (dlpf == ICM20948_DLPF_OFF)
    {
        regVal &= 0xFE;
        writeRegister8 (Bank_2, ICM20948_GYRO_CONFIG_1, regVal);
        return;
    }
    else
    {
        regVal |= 0x01;
        regVal &= 0xC7;
        regVal |= (uint8_t) (dlpf << 3);
    }
    writeRegister8 (Bank_2, ICM20948_GYRO_CONFIG_1, regVal);
}

/**************************************************************************************
 * @brief     Set Gyroscope Sample Rate Divider
 * @param[in] gyrSplRateDiv  Sample Rate Divider Value
 * @retval
 **************************************************************************************/

void setGyrSampleRateDivider(uint8_t gyrSplRateDiv)
{
    writeRegister8 (Bank_2, ICM20948_GYRO_SMPLRT_DIV, gyrSplRateDiv);
}

/**************************************************************************************
 * @brief     Get Accelerometer raw data
 * @param[in] a pointer to axis structure
 * @retval
 **************************************************************************************/

void getAccRawValues(xyzFloat *a)
{
    a->x = (int16_t) (((AccelResultsBuffer[0]) << 8) | AccelResultsBuffer[1]) * 1.0;
    a->y = (int16_t) (((AccelResultsBuffer[2]) << 8) | AccelResultsBuffer[3]) * 1.0;
    a->z = (int16_t) (((AccelResultsBuffer[4]) << 8) | AccelResultsBuffer[5]) * 1.0;
}

/**************************************************************************************
 * @brief     Get Corrected Accelerometer raw data
 * @param[in] a pointer to axis structure
 * @retval
 **************************************************************************************/
void getCorrectedAccRawValues(xyzFloat *a)
{
    getAccRawValues (a);
    correctAccRawValues (a);
}

/**************************************************************************************
 * @brief     Get  Accelerometer  data
 * @param[in] a pointer to axis structure
 * @retval
 **************************************************************************************/
void getGValues(xyzFloat *a)
{
    xyzFloat accRawVal;
    getCorrectedAccRawValues (&accRawVal);

    a->x = accRawVal.x * accRangeFactor / 16384.0;
    a->y = accRawVal.y * accRangeFactor / 16384.0;
    a->z = accRawVal.z * accRangeFactor / 16384.0;
}


/**************************************************************************************
 * @brief     Get  Accelerometer Raw  data from FIFO
 * @param[in] a pointer to axis structure
 * @retval
 **************************************************************************************/

void getCorrectedAccRawValuesFromFifo(xyzFloat *a)
{
    uint8_t MSByte = 0, LSByte = 0;
    //xyzFloat xyzResult = {0.0, 0.0, 0.0};
    MSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    LSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    a->x = ((int16_t) ((MSByte << 8) + LSByte)) * 1.0;
    MSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    LSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    a->y = ((int16_t) ((MSByte << 8) + LSByte)) * 1.0;
    MSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    LSByte = readRegister8 (0, ICM20948_FIFO_R_W);
    a->z = ((int16_t) ((MSByte << 8) + LSByte)) * 1.0;

    correctAccRawValues (a);
}

/**************************************************************************************
 * @brief     Get  Resultant G force
 * @param[in] gVal1     pointer to axis structure
 * @retval    resultant Resultant G force
 **************************************************************************************/

double getResultantG(xyzFloat gVal1)
{
    double resultant = 0.0;
    resultant = sqrt (sq(gVal1.x) + sq(gVal1.y) + sq(gVal1.z));
    return resultant;
}

/**************************************************************************************
 * @brief     Get  Gyroscope Raw Values
 * @param[in] a    pointer to axis structure
 * @retval
 **************************************************************************************/

void getGyrRawValues(xyzFloat *a)
{
    a->x = (int16_t) ((int16_t) ((AccelResultsBuffer[6]) << 8) | AccelResultsBuffer[7]) * 1.0;
    a->y = (int16_t) ((int16_t) ((AccelResultsBuffer[8]) << 8) | AccelResultsBuffer[9]) * 1.0;
    a->z = (int16_t) ((int16_t) ((AccelResultsBuffer[10]) << 8) | AccelResultsBuffer[11]) * 1.0;
}

/**************************************************************************************
 * @brief     Get corrected Gyroscope raw value
 * @param[in] a    pointer to axis structure
 * @retval
 **************************************************************************************/
void getCorrectedGyrRawValues(xyzFloat *a)
{
    getGyrRawValues (a);
    correctGyrRawValues (a);
}

/**************************************************************************************
 * @brief     Get  Gyroscope  value
 * @param[in] a    pointer to axis structure
 * @retval
 **************************************************************************************/
void getGyrValues(xyzFloat *a)
{
    getCorrectedGyrRawValues (a);

    a->x = a->x * gyrRangeFactor * 250.0 / 32768.0;
    a->y = a->y * gyrRangeFactor * 250.0 / 32768.0;
    a->z = a->z * gyrRangeFactor * 250.0 / 32768.0;
}
/**************************************************************************************
 * @brief     Get  Magnometer  value
 * @param[in] a    pointer to axis structure
 * @retval
 **************************************************************************************/
void getMagValues(xyzFloat *a)
{
    int16_t x, y, z;

    x = (int16_t) ((int16_t) ((AccelResultsBuffer[15]) << 8) | AccelResultsBuffer[14]);
    y = (int16_t) ((int16_t) ((AccelResultsBuffer[17]) << 8) | AccelResultsBuffer[16]);
    z = (int16_t) ((int16_t) ((AccelResultsBuffer[19]) << 8) | AccelResultsBuffer[18]);

    a->x = x * AK09916_MAG_LSB;
    a->y = y * AK09916_MAG_LSB;
    a->z = z * AK09916_MAG_LSB;
}

/********* Power, Sleep, Standby *********/

/**************************************************************************************
 * @brief     Enable  Power  Cycle
 * @param[in]  cycle
 * @retval
 **************************************************************************************/
void enableCycle(ICM20948_cycle cycle)
{
    regVal = readRegister8 (0, ICM20948_LP_CONFIG);
    regVal &= 0x0F;
    regVal |= cycle;
    writeRegister8 (0, ICM20948_LP_CONFIG, regVal);
}

/**************************************************************************************
 * @brief     Enable Low Power mode
 * @param[in]  enLP enable low power
 * @retval
 **************************************************************************************/
void enableLowPower(bool enLP)
{
    regVal = readRegister8 (0, ICM20948_PWR_MGMT_1);
    if (enLP)
    {
        regVal |= ICM20948_LP_EN;
    }
    else
    {
        regVal &= (uint8_t) (~ICM20948_LP_EN );
    }
    writeRegister8 (0, ICM20948_PWR_MGMT_1, regVal);
}

/**************************************************************************************
 * @brief     Enable Sensor setIcmSleepMode
 * @param[in]  sleep enable setIcmSleepMode mode
 * @retval
 **************************************************************************************/
static void setIcmSleepMode(bool sleep)
{
    regVal = readRegister8 (0, ICM20948_PWR_MGMT_1);
    if (sleep)
    {
        regVal |= ICM20948_SLEEP;
    }
    else
    {
        regVal &= (uint8_t) (~ICM20948_SLEEP );
    }
    writeRegister8 (0, ICM20948_PWR_MGMT_1, regVal);
}

/******** Angles and Orientation *********/
/**************************************************************************************
 * @brief    Get Angles
 * @param[in]  a  pointer to axis structure
 * @retval
 **************************************************************************************/
void getAngles(xyzFloat *a)
{

    xyzFloat _gVal;
    getGValues (&_gVal);
    if (_gVal.x > 1.0)
    {
        _gVal.x = 1.0;
    }
    else if (_gVal.x < -1.0)
    {
        _gVal.x = -1.0;
    }
    a->x = (asin (_gVal.x)) * 57.296;

    if (_gVal.y > 1.0)
    {
        _gVal.y = 1.0;
    }
    else if (_gVal.y < -1.0)
    {
        _gVal.y = -1.0;
    }
    a->y = (asin (_gVal.y)) * 57.296;

    if (_gVal.z > 1.0)
    {
        _gVal.z = 1.0;
    }
    else if (_gVal.z < -1.0)
    {
        _gVal.z = -1.0;
    }
    a->z = (asin (_gVal.z)) * 57.296;
}

/**************************************************************************************
 * @brief    Get Orientation
 * @param[in]
 * @retval orientation
 **************************************************************************************/

ICM20948_orientation getOrientation(void)
{
    xyzFloat angleVal;
    getAngles (&angleVal);
    ICM20948_orientation orientation = ICM20948_FLAT;
    if ((uint32_t) (angleVal.x) < (uint32_t) 45)
    {      // |x| < 45
        if ((uint32_t) (angleVal.y) < (uint32_t) 45)
        {      // |y| < 45
            if (angleVal.z > 0)
            {          //  z  > 0
                orientation = ICM20948_FLAT;
            }
            else
            {                        //  z  < 0
                orientation = ICM20948_FLAT_1;
            }
        }
        else
        {                         // |y| > 45
            if (angleVal.y > 0)
            {         //  y  > 0
                orientation = ICM20948_XY;
            }
            else
            {                       //  y  < 0
                orientation = ICM20948_XY_1;
            }
        }
    }
    else
    {                           // |x| >= 45
        if (angleVal.x > 0)
        {           //  x  >  0
            orientation = ICM20948_YX;
        }
        else
        {                       //  x  <  0
            orientation = ICM20948_YX_1;
        }
    }
    return orientation;
}

/**************************************************************************************
 * @brief    Get Pitch value
 * @param[in]
 * @retval pitch  pitch value
 **************************************************************************************/
float getPitch(void)
{
    xyzFloat angleVal;
    getAngles (&angleVal);
    float pitch = (float) ((atan2 (
            angleVal.x,
            (double) (sqrt (abs ((int) (angleVal.x * angleVal.y + angleVal.z * angleVal.z)))) * (double) 180.0) / M_PI));
    return pitch;
}

/**************************************************************************************
 * @brief    Get Roll value
 * @param[in]
 * @retval roll  pitch value
 **************************************************************************************/
float getRoll()
{
    xyzFloat angleVal;
    getAngles (&angleVal);
    float roll = (float) ((atan2 (angleVal.y, angleVal.z) * 180.0) / M_PI);
    return roll;
}

/************** Interrupts ***************/
/**************************************************************************************
 * @brief   Set Interrupt  Pin Polarity
 * @param[in] pol polarity
 * @retval
 **************************************************************************************/
void setIntPinPolarity(ICM20948_intPinPol pol)
{
    regVal = readRegister8 (0, ICM20948_INT_PIN_CFG);
    if (pol)
    {
        regVal |= ICM20948_INT1_ACTL;
    }
    else
    {
        regVal &= (uint8_t) ~ICM20948_INT1_ACTL;
    }
    writeRegister8 (0, ICM20948_INT_PIN_CFG, regVal);
}

/**************************************************************************************
 * @brief   Enable Interrupt Latch
 * @param[in] latch
 * @retval
 **************************************************************************************/
void enableIntLatch(bool latch)
{
    regVal = readRegister8 (0, ICM20948_INT_PIN_CFG);
    if (latch)
    {
        regVal |= ICM20948_INT_1_LATCH_EN;
    }
    else
    {
        regVal &= (uint8_t) ~ICM20948_INT_1_LATCH_EN;
    }
    writeRegister8 (0, ICM20948_INT_PIN_CFG, regVal);
}

/**************************************************************************************
 * @brief   Enable Clear Interrupt by Any read
 * @param[in] clearByAnyRead
 * @retval
 **************************************************************************************/
void enableClearIntByAnyRead(bool clearByAnyRead)
{
    regVal = readRegister8 (0, ICM20948_INT_PIN_CFG);
    if (clearByAnyRead)
    {
        regVal |= ICM20948_INT_ANYRD_2CLEAR;
    }
    else
    {
        regVal &= (uint8_t) ~ICM20948_INT_ANYRD_2CLEAR;
    }
    writeRegister8 (0, ICM20948_INT_PIN_CFG, regVal);
}

/**************************************************************************************
 * @brief   setFSyncIntPolarity
 * @param[in] pol polarity
 * @retval
 **************************************************************************************/
void setFSyncIntPolarity(ICM20948_intPinPol pol)
{
    regVal = readRegister8 (0, ICM20948_INT_PIN_CFG);
    if (pol)
    {
        regVal |= ICM20948_ACTL_FSYNC;
    }
    else
    {
        regVal &= (uint8_t) ~ICM20948_ACTL_FSYNC;
    }
    writeRegister8 (0, ICM20948_INT_PIN_CFG, regVal);
}

/**************************************************************************************
 * @brief   Enable Interrupt
 * @param[in] intType Interrupt type
 * @retval
 **************************************************************************************/
void enableInterrupt(ICM20948_intType intType)
{
    switch (intType)
    {
        case ICM20948_FSYNC_INT:
            regVal = readRegister8 (0, ICM20948_INT_PIN_CFG);
            regVal |= ICM20948_FSYNC_INT_MODE_EN;
            writeRegister8 (0, ICM20948_INT_PIN_CFG, regVal); // enable FSYNC as interrupt pin
            regVal = readRegister8 (0, ICM20948_INT_ENABLE);
            regVal |= 0x80;
            writeRegister8 (0, ICM20948_INT_ENABLE, regVal); // enable wake on FSYNC interrupt
        break;

        case ICM20948_WOM_INT:
            regVal = readRegister8 (0, ICM20948_INT_ENABLE);
            regVal |= 0x08;
            writeRegister8 (0, ICM20948_INT_ENABLE, regVal);
            regVal = readRegister8 (2, ICM20948_ACCEL_INTEL_CTRL);
            regVal |= 0x02;
            writeRegister8 (2, ICM20948_ACCEL_INTEL_CTRL, regVal);
        break;

        case ICM20948_DMP_INT:
            regVal = readRegister8 (0, ICM20948_INT_ENABLE);
            regVal |= 0x02;
            writeRegister8 (0, ICM20948_INT_ENABLE, regVal);
        break;

        case ICM20948_DATA_READY_INT:
            writeRegister8 (0, ICM20948_INT_ENABLE_1, 0x01);
        break;

        case ICM20948_FIFO_OVF_INT:
            writeRegister8 (0, ICM20948_INT_ENABLE_2, 0x01);
        break;

        case ICM20948_FIFO_WM_INT:
            writeRegister8 (0, ICM20948_INT_ENABLE_3, 0x01);
        break;
    }
}

/**************************************************************************************
 * @brief   Disable Interrupt
 * @param[in] intType Interrupt type
 * @retval
 **************************************************************************************/
void disableInterrupt(ICM20948_intType intType)
{
    switch (intType)
    {
        case ICM20948_FSYNC_INT:
            regVal = readRegister8 (Bank_0, (uint8_t) ICM20948_INT_PIN_CFG);
            regVal = (uint8_t) (regVal & (~(uint8_t) (ICM20948_FSYNC_INT_MODE_EN )));
            writeRegister8 (Bank_0, ICM20948_INT_PIN_CFG, regVal);
            regVal = readRegister8 (Bank_0, (uint8_t) ICM20948_INT_ENABLE);
            regVal = (uint8_t) (regVal & (~(uint8_t) (ICM20948_INT1_ACTL )));
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE, regVal);
        break;

        case ICM20948_WOM_INT:
            regVal = readRegister8 (Bank_0, (uint8_t) ICM20948_INT_ENABLE);
            regVal = (uint8_t) (regVal & ~(ICM20948_ZG_OFFS_USRL ));
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE, regVal);
            regVal = readRegister8 (Bank_2, (uint8_t) ICM20948_ACCEL_INTEL_CTRL);
            regVal = (uint8_t) (regVal & (~(ICM20948_GYRO_CONFIG_2 )));
            writeRegister8 (Bank_2, ICM20948_ACCEL_INTEL_CTRL, regVal);
        break;

        case ICM20948_DMP_INT:
            regVal = readRegister8 (Bank_0, ICM20948_INT_ENABLE);
            regVal = (uint8_t) (regVal & ~(ICM20948_GYRO_CONFIG_2 ));
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE, regVal);
        break;

        case ICM20948_DATA_READY_INT:
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE_1, 0x00);
        break;

        case ICM20948_FIFO_OVF_INT:
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE_2, 0x00);
        break;

        case ICM20948_FIFO_WM_INT:
            writeRegister8 (Bank_0, ICM20948_INT_ENABLE_3, 0x00);
        break;
    }
}

/**************************************************************************************
 * @brief   Read And Clear Interrupts
 * @param[in]
 * @retval intSource Interrupt Source
 **************************************************************************************/
uint8_t readAndClearInterrupts(void)
{
    uint8_t intSource = 0;
    regVal = readRegister8 (Bank_0, ICM20948_I2C_MST_STATUS);
    if (regVal & 0x80)
    {
        intSource |= 0x01;
    }
    regVal = readRegister8 (0, ICM20948_INT_STATUS);
    if (regVal & 0x08)
    {
        intSource |= 0x02;
    }
    if (regVal & 0x02)
    {
        intSource |= 0x04;
    }
    regVal = readRegister8 (0, ICM20948_INT_STATUS_1);
    if (regVal & 0x01)
    {
        intSource |= 0x08;
    }
    regVal = readRegister8 (0, ICM20948_INT_STATUS_2);
    if (regVal & 0x01)
    {
        intSource |= 0x10;
    }
    regVal = readRegister8 (0, ICM20948_INT_STATUS_3);
    if (regVal & 0x01)
    {
        intSource |= 0x20;
    }
    return intSource;
}

/**************************************************************************************
 * @brief   Check Interrupts
 * @param[in]   'source' Interrupt Source,
 * @param[in]   'type' Interrupt type
 * @retval      source Interrupt Source
 **************************************************************************************/
bool checkInterrupt(uint8_t source, ICM20948_intType type)
{
    source &= type;
    return source;
}

/**************************************************************************************
 * @brief   Set WakeOn MotionThreshold
 * @param[in]   'womThresh' Threshold level,
 * @param[in]   'womCompEn' Comparison Enable
 * @retval
 **************************************************************************************/

void setWakeOnMotionThreshold(uint8_t womThresh, ICM20948_womCompEn womCompEn)
{
    regVal = readRegister8 (2, ICM20948_ACCEL_INTEL_CTRL);
    if (womCompEn)
    {
        regVal |= 0x01;
    }
    else
    {
        regVal = (uint8_t) (regVal & (~(0x01)));
    }
    writeRegister8 (Bank_2, ICM20948_ACCEL_INTEL_CTRL, regVal);
    writeRegister8 (Bank_2, ICM20948_ACCEL_WOM_THR, womThresh);
}

/***************** FIFO ******************/
/**************************************************************************************
 * @brief   Enable Fifo
 * @param[in]   'fifo'  bool
 * @param[in]
 * @retval
 **************************************************************************************/
void enableFifo(bool fifo)
{
    regVal = readRegister8 (Bank_0, ICM20948_USER_CTRL);
    if (fifo)
    {
        regVal |= ICM20948_FIFO_EN;
    }
    else
    {
        regVal = (uint8_t) (regVal & (~ICM20948_FIFO_EN ));
    }
    writeRegister8 (Bank_0, ICM20948_USER_CTRL, regVal);
}

/**************************************************************************************
 * @brief   SetFifoMode
 * @param[in]   'mode' fifoMode enumeration
 * @param[in]
 * @retval
 **************************************************************************************/
void setFifoMode(ICM20948_fifoMode mode)
{
    if (mode)
    {
        regVal = 0x01;
    }
    else
    {
        regVal = 0x00;
    }
    writeRegister8 (Bank_0, ICM20948_FIFO_MODE, regVal);
}

/**************************************************************************************
 * @brief   Start Fifo
 * @param[in]   'fifo' fifoType enumeration
 * @param[in]
 * @retval
 **************************************************************************************/
void startFifo(ICM20948_fifoType fifo)
{
    fifoType = fifo;
    writeRegister8 (Bank_0, ICM20948_FIFO_EN_2, fifoType);
}

/**************************************************************************************
 * @brief  Stop  Fifo
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
void stopFifo(void)
{
    writeRegister8 (Bank_0, ICM20948_FIFO_EN_2, 0);
}

/**************************************************************************************
 * @brief   Reset Fifo
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
void resetFifo(void)
{
    writeRegister8 (Bank_0, ICM20948_FIFO_RST, 0x01);
    writeRegister8 (Bank_0, ICM20948_FIFO_RST, 0x00);
}

/**************************************************************************************
 * @brief  Get Fifo count
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
int16_t getFifoCount(void)
{
    int16_t regVal16 = (int16_t) readRegister16 (0, ICM20948_FIFO_COUNT);
    return regVal16;
}

/**************************************************************************************
 * @brief  Get Number Of Fifo Data Sets
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
int16_t getNumberOfFifoDataSets(void)
{
    int16_t numberOfSets = getFifoCount ();

    if ((fifoType == ICM20948_FIFO_ACC) || (fifoType == ICM20948_FIFO_GYR))
    {
        numberOfSets /= 6;
    }
    else if (fifoType == ICM20948_FIFO_ACC_GYR)
    {
        numberOfSets /= 12;
    }

    return numberOfSets;
}
/**************************************************************************************
 * @brief  Find Fifo Begin
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
void findFifoBegin(void)
{
    int16_t count = getFifoCount ();
    int16_t start = 0;

    if ((fifoType == ICM20948_FIFO_ACC) || (fifoType == ICM20948_FIFO_GYR))
    {
        start = count % 6;
        for (int i = 0; i < start; i++)
        {
            readRegister8 (0, ICM20948_FIFO_R_W);
        }
    }
    else if (fifoType == ICM20948_FIFO_ACC_GYR)
    {
        start = count % 12;
        for (int i = 0; i < start; i++)
        {
            readRegister8 (0, ICM20948_FIFO_R_W);
        }
    }
}

/************** Magnetometer **************/
/**************************************************************************************
 * @brief  Initialize Magnetometer
 * @param[in]
 * @param[in]
 * @retval  True
 **************************************************************************************/
void initSlaveMagnetometer(void)
{
    /* enable I2C master */
    writeRegister8 (0, ICM20948_USER_CTRL, ICM20948_I2C_MST_EN);
    /* set I2C clock to 345.60 kHz */
    writeRegister8 (3, ICM20948_I2C_MST_CTRL, 0x07);
    vTaskDelay (10);

    /* Reset Magnetometer */
    writeAK09916Register8 (AK09916_CNTL_3, 0x01);
    vTaskDelay (100);

    requestIcmReset();
    setIcmSleepMode(false);
    writeRegister8 (2, ICM20948_ODR_ALIGN_EN, 1); // aligns ODR

    /* Reenable I2C master, since we just the sensor */
    writeRegister8 (0, ICM20948_USER_CTRL, ICM20948_I2C_MST_EN);
    /* set I2C clock to 345.60 kHz */
    writeRegister8 (3, ICM20948_I2C_MST_CTRL, 0x07);
    vTaskDelay (10);

    if (readAK09916Register16 (AK09916_WIA_1) != AK09916_WHO_AM_I)
    {
        //TODO log error, unexpected Magnetometer ID
    }
}


/**************************************************************************************
 * @brief Set Magnotometer Operation Mode
 * @param[in] opMode  Operation mode enumeration
 * @param[in]
 * @retval
 **************************************************************************************/
void setMagOpMode(AK09916_opMode opMode)
{
    writeAK09916Register8 (AK09916_CNTL_2, opMode);
    vTaskDelay (10);
    if (opMode != AK09916_PWR_DOWN)
    {
        enableMagDataRead (AK09916_HXL, 0x08);
    }
}


/************************************************
 Private Functions
 *************************************************/
/**************************************************************************************
 * @brief Set Clock To Auto Select
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
void setClockToAutoSelect(void)
{
    regVal = readRegister8 (0, ICM20948_PWR_MGMT_1);
    regVal |= 0x01;
    writeRegister8 (0, ICM20948_PWR_MGMT_1, regVal);
    vTaskDelay (10);
}

/**************************************************************************************
 * @brief Correct Acc Raw Values
 * @param[in] a pointer to axis structure
 * @param[in]
 * @retval
 **************************************************************************************/

void correctAccRawValues(xyzFloat *a)
{
    a->x = (a->x - (accOffsetVal.x / accRangeFactor)) / accCorrFactor.x;
    a->y = (a->y - (accOffsetVal.y / accRangeFactor)) / accCorrFactor.y;
    a->z = (a->z - (accOffsetVal.z / accRangeFactor)) / accCorrFactor.z;
}

/**************************************************************************************
 * @brief Correct Accelerometer Raw Values
 * @param[in] a pointer to axis structure
 * @param[in]
 * @retval
 **************************************************************************************/
void correctGyrRawValues(xyzFloat *a)
{
    a->x -= (gyrOffsetVal.x / gyrRangeFactor);
    a->y -= (gyrOffsetVal.y / gyrRangeFactor);
    a->z -= (gyrOffsetVal.z / gyrRangeFactor);
}

/**************************************************************************************
 * @brief Switch Bank
 * @param[in] newBank
 * @param[in]
 * @retval
 **************************************************************************************/
void switchBank(uint8_t newBank)
{
    if (newBank != currentBank)
    {
        currentBank = newBank;
        currentBank = (uint8_t) (currentBank << 4);
        writeDevice(ICM20948_REG_BANK_SEL, &currentBank, (uint8_t) 1);
    }
}

/**************************************************************************************
 * @brief Write Register 8
 * @param[in] bank
 * @param[in] reg
 * @param[in] val
 * @retval    Ret  FSP_SUCCESS or Error code
 **************************************************************************************/
fsp_err_t writeRegister8(uint8_t bank, uint8_t reg, uint8_t val)
{
    switchBank (bank);
    fsp_err_t Ret = 0;
    Ret = writeDevice(reg, &val, (uint8_t) 1);
    return Ret;
}

/**************************************************************************************
 * @brief Write Register 16
 * @param[in] bank
 * @param[in] reg
 * @param[in] val
 * @retval    Ret FSP_SUCCESS or Error code
 **************************************************************************************/
fsp_err_t writeRegister16(uint8_t bank, uint8_t reg, uint16_t val)
{
    fsp_err_t Ret = 0;
    switchBank (bank);
    uint8_t MSByte = (uint8_t) ((val >> 8) & 0xFF);
    uint8_t LSByte = (uint8_t) (val & 0xFF);
    uint8_t WriteByte[2];
    WriteByte[0] = (uint8_t) MSByte;
    WriteByte[1] = LSByte;
    Ret = writeDevice(reg, WriteByte, (uint8_t) 2);
    return Ret;
}

/**************************************************************************************
 * @brief Read Register 8
 * @param[in] bank
 * @param[in] reg
 * @param[in]
 * @retval   regValue regeister value
 **************************************************************************************/
uint8_t readRegister8(uint8_t bank, uint8_t reg)
{
    switchBank (bank);
    uint8_t regValue = 0;
    readDevice(reg, &regValue, (uint8_t) 1);
    return regValue;
}

/**************************************************************************************
 * @brief Read Register 16
 * @param[in] bank
 * @param[in] reg
 * @param[in]
 * @retval   reg16Val regeister value
 **************************************************************************************/
int16_t readRegister16(uint8_t bank, uint8_t reg)
{
    switchBank (bank);
    uint8_t MSByte = 0, LSByte = 0, regValue[2];
    int16_t reg16Val = 0;
    readDevice(reg, regValue, 2);
    MSByte = regValue[0];
    LSByte = regValue[1];
    reg16Val = (uint8_t) (((uint16_t) (MSByte << 8)) + LSByte);
    return reg16Val;
}

/**************************************************************************************
 * @brief Read All data
 * @param[in] data
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
static void readAccelResults(uint8_t *data)
{
    switchBank (0);
    readDevice(ICM20948_ACCEL_OUT, data, 20);
}



/**************************************************************************************
 * @brief Read ICM20948 xyz axis Values From Fifo
 * @param[in] a pointer to axis structure
 * @param[in]
 * @param[in]
 * @retval
 **************************************************************************************/
static void writeAK09916Register8(uint8_t reg, uint8_t val)
{
    writeRegister8 (3, ICM20948_I2C_SLV0_ADDR, AK09916_ADDRESS); // write AK09916
    writeRegister8 (3, ICM20948_I2C_SLV0_REG, reg); // define AK09916 register to be written to
    writeRegister8 (3, ICM20948_I2C_SLV0_DO, val);
}

/**************************************************************************************
 * @brief Read AK09916 Register 8
 * @param[in]reg
 * @param[in]
 * @param[in]
 * @retval  regVal
 **************************************************************************************/
static uint8_t readAK09916Register8(uint8_t reg)
{
    enableMagDataRead (reg, 0x01);
    enableMagDataRead (AK09916_HXL, 0x08);
    regVal = readRegister8 (0, ICM20948_EXT_SLV_SENS_DATA_00);
    return regVal;
}

/**************************************************************************************
 * @brief Read AK09916 Register 16
 * @param[in]reg
 * @param[in]
 * @param[in]
 * @retval   regValue
 **************************************************************************************/
static int16_t readAK09916Register16(uint8_t reg)
{
    int16_t regValue = 0;
    enableMagDataRead (reg, 0x02);
    regValue = readRegister16 (0, ICM20948_EXT_SLV_SENS_DATA_00);
    enableMagDataRead (AK09916_HXL, 0x08);
    return regValue;
}

/**************************************************************************************
 * @brief Read ICM20948
 * @param[in]
 * @param[in]
 * @param[in]
 * @retval True if Write Reg Successfully Or False
 **************************************************************************************/
static uint8_t requestIcmReset(void)
{
    uint8_t ack = writeRegister8 (0, ICM20948_PWR_MGMT_1, ICM20948_RESET);
    /* wait for registers to reset */
    vTaskDelay (50);
    return (ack == 0);
}

/**************************************************************************************
 * @brief Enable Magnetometer Data Read
 * @param[in] reg
 * @param[in] bytes
 * @param[in]
 * @retval
 **************************************************************************************/
static void enableMagDataRead(uint8_t reg, uint8_t bytes)
{
    /* read AK09916 */
    writeRegister8 (3, ICM20948_I2C_SLV0_ADDR, AK09916_ADDRESS | AK09916_READ);
    /* define AK09916 register to be read */
    writeRegister8 (3, ICM20948_I2C_SLV0_REG, reg);
    /* enable read | number of byte */
    writeRegister8 (3, ICM20948_I2C_SLV0_CTRL, 0x80 | bytes);
    vTaskDelay (10);
}

/*******************************************************************************************************************//**
 * @brief       I2C callback for ICM sensor
 * @param[in]   p_args
 * @retval
 * @retval
 ***********************************************************************************************************************/
void SensorIcm20948_CommCallback(rm_comms_callback_args_t *p_args)
{
    if (p_args->event == RM_COMMS_EVENT_OPERATION_COMPLETE)
    {
        transmit_complete_flag = I2C_TRANSMISSION_COMPLETE;
    }
    else
    {
        transmit_complete_flag = I2C_TRANSMISSION_ABORTED;
    }
}

void SensorIcm20948_Init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t icm20948_id;

    /* Init I2C device with appropriate FSP calls  */
    err = g_comms_i2c_device5.p_api->open (g_comms_i2c_device5.p_ctrl, g_comms_i2c_device5.p_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_DBG_PRINT("\r\nICM20948 sensor open failed: %u\r\n", err);
        APP_ERR_TRAP(err); //TODO revamp log messages
    }
    else
    {
        APP_PRINT("\r\nICM20948 sensor setup success\r\n");
    }

    /* Check if ID number of sensor is the expected 0xEA */
    if((err == FSP_SUCCESS) &&
        (readRegister8 (0, ICM20948_WHO_AM_I) != 0xEA))
    {
        err = FSP_ERR_NOT_FOUND;
    }

    requestIcmReset();
    fifoType = ICM20948_FIFO_ACC;
    setIcmSleepMode(false);
    writeRegister8 (2, ICM20948_ODR_ALIGN_EN, 1);
    writeRegister8 (2, ICM20948_ODR_ALIGN_EN, 1);

    initSlaveMagnetometer();
    setMagOpMode (AK09916_CONT_MODE_20HZ);
    vTaskDelay(pdMS_TO_TICKS(1000u));

    configIcmOffsets();
    setAccRange (ICM20948_ACC_RANGE_4G);
    setAccDLPF (ICM20948_DLPF_6);
    setAccSampleRateDivider (10);
    setGyrRange (ICM20948_GYRO_RANGE_250);
    setGyrDLPF (ICM20948_DLPF_6);
}

void SensorIcm20948_MainFunction(void)
{
    readAccelResults(AccelResultsBuffer);
    getCorrectedAccRawValues (&corrAccRaw);
    getGValues (&gVal);
    resultantG = (float) getResultantG (gVal);
    getGyrValues (&gyr);
    getMagValues (&magValue); // returns magnetic flux density [µT]
}

void SensorIcm20948_GetData(xyzFloat *acc, xyzFloat *gval, xyzFloat *magnitude)
{
    *acc = corrAccRaw;
    *gval = gVal;
    *magnitude = magValue;
}