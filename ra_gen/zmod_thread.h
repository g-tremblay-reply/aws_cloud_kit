/* generated thread header file - do not edit */
#ifndef ZMOD_THREAD_H_
#define ZMOD_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void zmod_thread_entry(void * pvParameters);
                #else
extern void zmod_thread_entry(void *pvParameters);
#endif
#include "rm_comms_i2c.h"
#include "rm_comms_api.h"
#include "../ra/fsp/src/rm_zmod4xxx/zmod4xxx_types.h"
#include "../ra/fsp/src/rm_zmod4xxx/iaq_1st_gen/iaq_1st_gen.h"
#include "rm_zmod4xxx.h"
#include "rm_zmod4xxx_api.h"
FSP_HEADER
/* I2C Communication Device */
extern const rm_comms_instance_t g_comms_i2c_device1;
extern rm_comms_i2c_instance_ctrl_t g_comms_i2c_device1_ctrl;
extern const rm_comms_cfg_t g_comms_i2c_device1_cfg;
#ifndef rm_zmod4xxx_comms_i2c_callback
void rm_zmod4xxx_comms_i2c_callback(rm_comms_callback_args_t *p_args);
#endif
/* ZMOD4410 IAQ 1st Gen. */
extern rm_zmod4xxx_lib_extended_cfg_t g_zmod4xxx_sensor0_extended_cfg;
/* ZMOD4XXX Sensor */
extern const rm_zmod4xxx_instance_t g_zmod4xxx_sensor0;
extern rm_zmod4xxx_instance_ctrl_t g_zmod4xxx_sensor0_ctrl;
extern const rm_zmod4xxx_cfg_t g_zmod4xxx_sensor0_cfg;
#ifndef zmod4xxx_comms_i2c_callback
void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t *p_args);
#endif
#ifndef rm_zmod4xxx_irq_callback
void rm_zmod4xxx_irq_callback(external_irq_callback_args_t *p_args);
#endif
#ifndef zmod4xxx_irq0_callback
void zmod4xxx_irq0_callback(rm_zmod4xxx_callback_args_t *p_args);
#endif
FSP_FOOTER
#endif /* ZMOD_THREAD_H_ */
