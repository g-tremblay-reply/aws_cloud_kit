/* generated thread header file - do not edit */
#ifndef OXIMETER_THREAD_H_
#define OXIMETER_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void oximeter_thread_entry(void * pvParameters);
                #else
extern void oximeter_thread_entry(void *pvParameters);
#endif
#include "rm_comms_i2c.h"
#include "rm_comms_api.h"
#include "rm_ob1203.h"
#include "rm_ob1203_api.h"
FSP_HEADER
/* I2C Communication Device */
extern const rm_comms_instance_t g_comms_i2c_device6;
extern rm_comms_i2c_instance_ctrl_t g_comms_i2c_device6_ctrl;
extern const rm_comms_cfg_t g_comms_i2c_device6_cfg;
#ifndef rm_ob1203_comms_i2c_callback
void rm_ob1203_comms_i2c_callback(rm_comms_callback_args_t *p_args);
#endif
/* OB1203 Proximity mode */
extern rm_ob1203_mode_extended_cfg_t g_ob1203_sensor1_extended_cfg;
/* OB1203 Sensor */
extern const rm_ob1203_instance_t g_ob1203_sensor1;
extern rm_ob1203_instance_ctrl_t g_ob1203_sensor1_ctrl;
extern const rm_ob1203_cfg_t g_ob1203_sensor1_cfg;
#ifndef ob1203_comms_i2c_callback
void ob1203_comms_i2c_callback(rm_ob1203_callback_args_t *p_args);
#endif
#ifndef rm_ob1203_irq_callback
void rm_ob1203_irq_callback(external_irq_callback_args_t *p_args);
#endif
#ifndef ob1203_irq_callback
void ob1203_irq_callback(rm_ob1203_callback_args_t *p_args);
#endif
/* I2C Communication Device */
extern const rm_comms_instance_t g_comms_i2c_device3;
extern rm_comms_i2c_instance_ctrl_t g_comms_i2c_device3_ctrl;
extern const rm_comms_cfg_t g_comms_i2c_device3_cfg;
#ifndef rm_ob1203_comms_i2c_callback
void rm_ob1203_comms_i2c_callback(rm_comms_callback_args_t *p_args);
#endif
/* OB1203 PPG mode */
extern rm_ob1203_mode_extended_cfg_t g_ob1203_sensor0_extended_cfg;
/* OB1203 Sensor */
extern const rm_ob1203_instance_t g_ob1203_sensor0;
extern rm_ob1203_instance_ctrl_t g_ob1203_sensor0_ctrl;
extern const rm_ob1203_cfg_t g_ob1203_sensor0_cfg;
#ifndef ob1203_comms_i2c_callback
void ob1203_comms_i2c_callback(rm_ob1203_callback_args_t *p_args);
#endif
#ifndef rm_ob1203_irq_callback
void rm_ob1203_irq_callback(external_irq_callback_args_t *p_args);
#endif
#ifndef ob1203_irq_callback
void ob1203_irq_callback(rm_ob1203_callback_args_t *p_args);
#endif
FSP_FOOTER
#endif /* OXIMETER_THREAD_H_ */
