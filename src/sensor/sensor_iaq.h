/***********************************************************************************************************************
 * File Name    : sensor_iaq.h
 * Description  : Contains global functions declarations defined in sensor_iaq.c
 ***********************************************************************************************************************/

#ifndef SENSOR_IAQ_H_
#define SENSOR_IAQ_H_

#include <zmod_thread.h>

void Sensor_IaqInit(void);
void Sensor_IaqMainFunction(void);
void Sensor_IaqGetData(rm_zmod4xxx_iaq_1st_data_t *data);



#endif /* SENSOR_IAQ_H_ */
