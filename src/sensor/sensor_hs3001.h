/***********************************************************************************************************************
 * File Name    : sensor_hs3001.h
 * Description  : Contains data structures and function prototypes for HS3001 sensor
 ***********************************************************************************************************************/

#ifndef SENSOR_HS3001_H_
#define SENSOR_HS3001_H_

void SensorHs3001_Init(void);
void SensorHs3001_MainFunction(void);
void SensorHs3001_GetData(float *temperature, float *humidity);

#endif /*SENSOR_HS3001_H_*/
