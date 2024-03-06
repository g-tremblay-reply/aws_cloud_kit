/* generated thread source file - do not edit */
#include "oximeter_thread.h"

#if 1
static StaticTask_t oximeter_thread_memory;
#if defined(__ARMCC_VERSION)           /* AC6 compiler */
                static uint8_t oximeter_thread_stack[2048] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
                #else
static uint8_t oximeter_thread_stack[2048] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.oximeter_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
#endif
#endif
TaskHandle_t oximeter_thread;
void oximeter_thread_create(void);
static void oximeter_thread_func(void *pvParameters);
void rtos_startup_err_callback(void *p_instance, void *p_data);
void rtos_startup_common_init(void);
/* I2C Communication Device */
#define RA_NOT_DEFINED (1)
rm_comms_i2c_instance_ctrl_t g_comms_i2c_device6_ctrl;

/* Lower level driver configuration */
const i2c_master_cfg_t g_comms_i2c_device6_lower_level_cfg = { .slave = 0x53,
		.addr_mode = I2C_MASTER_ADDR_MODE_7BIT, };

const rm_comms_cfg_t g_comms_i2c_device6_cfg = {
		.semaphore_timeout = 0xFFFFFFFF, .p_lower_level_cfg =
				(void*) &g_comms_i2c_device6_lower_level_cfg, .p_extend =
				(void*) &g_comms_i2c_bus0_extended_cfg, .p_callback =
				rm_ob1203_comms_i2c_callback,
#if defined(g_ob1203_sensor1_ctrl)
    .p_context          = g_ob1203_sensor1_ctrl,
#else
		.p_context = (void*) &g_ob1203_sensor1_ctrl,
#endif
		};

const rm_comms_instance_t g_comms_i2c_device6 = { .p_ctrl =
		&g_comms_i2c_device6_ctrl, .p_cfg = &g_comms_i2c_device6_cfg, .p_api =
		&g_comms_on_comms_i2c, };
extern rm_ob1203_api_t const g_ob1203_on_ob1203_proximity;
rm_ob1203_mode_extended_cfg_t g_ob1203_sensor1_extended_cfg = { .p_api =
		(void*) &g_ob1203_on_ob1203_proximity, .ppg_prox_gain =
		RM_OB1203_PPG_PROX_GAIN_1, .led_order =
		RM_OB1203_LED_IR_FIRST_RED_SECOND, .prox_sleep =
		RM_OB1203_SLEEP_AFTER_INTERRUPT_DISABLE, .prox_interrupt_type =
		RM_OB1203_PROX_INTERRUPT_TYPE_NORMAL, .prox_interrupt_persist = 0x02,
		.prox_led_current = 0x100, .prox_ana_can =
				RM_OB1203_ANALOG_CANCELLATION_DISABLE, .prox_dig_can = 0x100,
		.prox_num_led_pulses = RM_OB1203_NUM_LED_PULSES_8,
		.prox_upper_threshold = 0x0600, .prox_lower_threshold = 0x0000,
		.prox_width_period = RM_OB1203_PROX_WIDTH_42US_PERIOD_100MS,
		.prox_moving_average = RM_OB1203_MOVING_AVERAGE_DISABLE,
		.prox_hysteresis = 0x00, };
#if BSP_CFG_RTOS
#if BSP_CFG_RTOS == 1
TX_SEMAPHORE g_ob1203_sensor1_semaphore_handle;
CHAR g_ob1203_sensor1_semaphore_name[] = "g_ob1203_sensor1 semaphore";
#elif BSP_CFG_RTOS == 2
SemaphoreHandle_t g_ob1203_sensor1_semaphore_handle;
StaticSemaphore_t g_ob1203_sensor1_semaphore_memory;
#endif

/* Semaphore to wait for callback  */
rm_ob1203_semaphore_t g_ob1203_sensor1_semaphore =
{
    .p_semaphore_handle = &g_ob1203_sensor1_semaphore_handle,
#if BSP_CFG_RTOS == 1 // ThreadX
    .p_semaphore_name = &g_ob1203_sensor1_semaphore_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_semaphore_memory = &g_ob1203_sensor1_semaphore_memory,
#endif
};
#endif

rm_ob1203_instance_ctrl_t g_ob1203_sensor1_ctrl;
const rm_ob1203_cfg_t g_ob1203_sensor1_cfg = {
#if BSP_CFG_RTOS
    .p_semaphore = &g_ob1203_sensor1_semaphore,
#endif
		.semaphore_timeout = 0xFFFFFFFF, .p_comms_instance =
				&g_comms_i2c_device6,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_external_irq14)
    .p_irq_instance = NULL,
    .p_irq_callback = NULL,
#else
		.p_irq_instance = &g_external_irq14, .p_irq_callback =
				ob1203_irq_callback,
#endif
#undef RA_NOT_DEFINED
		.p_comms_callback = ob1203_comms_i2c_callback,
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = (void*) &g_ob1203_sensor1_extended_cfg, };

const rm_ob1203_instance_t g_ob1203_sensor1 = {
		.p_ctrl = &g_ob1203_sensor1_ctrl, .p_cfg = &g_ob1203_sensor1_cfg,
		.p_api = &g_ob1203_on_ob1203, };
/* I2C Communication Device */
#define RA_NOT_DEFINED (1)
rm_comms_i2c_instance_ctrl_t g_comms_i2c_device3_ctrl;

/* Lower level driver configuration */
const i2c_master_cfg_t g_comms_i2c_device3_lower_level_cfg = { .slave = 0x53,
		.addr_mode = I2C_MASTER_ADDR_MODE_7BIT, };

const rm_comms_cfg_t g_comms_i2c_device3_cfg = {
		.semaphore_timeout = 0xFFFFFFFF, .p_lower_level_cfg =
				(void*) &g_comms_i2c_device3_lower_level_cfg, .p_extend =
				(void*) &g_comms_i2c_bus0_extended_cfg, .p_callback =
				rm_ob1203_comms_i2c_callback,
#if defined(g_ob1203_sensor0_ctrl)
    .p_context          = g_ob1203_sensor0_ctrl,
#else
		.p_context = (void*) &g_ob1203_sensor0_ctrl,
#endif
		};

const rm_comms_instance_t g_comms_i2c_device3 = { .p_ctrl =
		&g_comms_i2c_device3_ctrl, .p_cfg = &g_comms_i2c_device3_cfg, .p_api =
		&g_comms_on_comms_i2c, };
extern rm_ob1203_api_t const g_ob1203_on_ob1203_ppg;
rm_ob1203_mode_extended_cfg_t g_ob1203_sensor0_extended_cfg = { .p_api =
		(void*) &g_ob1203_on_ob1203_ppg, .ppg_prox_gain =
		RM_OB1203_PPG_PROX_GAIN_1, .led_order =
		RM_OB1203_LED_IR_FIRST_RED_SECOND, .ppg_sensor_mode =
		RM_OB1203_PPG_SENSOR_MODE_PPG2, .ppg_interrupt_type =
		RM_OB1203_PPG_INTERRUPT_TYPE_DATA, .ppg_ir_led_current = 0x366,
		.ppg_red_led_current = 0x1B3, .ppg_power_save_mode =
				RM_OB1203_POWER_SAVE_MODE_DISABLE, .ppg_ir_led_ana_can =
				RM_OB1203_ANALOG_CANCELLATION_DISABLE, .ppg_red_led_ana_can =
				RM_OB1203_ANALOG_CANCELLATION_DISABLE,
		.ppg_num_averaged_samples = RM_OB1203_NUM_AVERAGED_SAMPLES_8,
		.ppg_width_period = RM_OB1203_PPG_WIDTH_130US_PERIOD_1P25MS,
		.ppg_fifo_rollover = RM_OB1203_FIFO_ROLLOVER_ENABLE,
		.ppg_fifo_empty_num = 0x0C, };
#if BSP_CFG_RTOS
#if BSP_CFG_RTOS == 1
TX_SEMAPHORE g_ob1203_sensor0_semaphore_handle;
CHAR g_ob1203_sensor0_semaphore_name[] = "g_ob1203_sensor0 semaphore";
#elif BSP_CFG_RTOS == 2
SemaphoreHandle_t g_ob1203_sensor0_semaphore_handle;
StaticSemaphore_t g_ob1203_sensor0_semaphore_memory;
#endif

/* Semaphore to wait for callback  */
rm_ob1203_semaphore_t g_ob1203_sensor0_semaphore =
{
    .p_semaphore_handle = &g_ob1203_sensor0_semaphore_handle,
#if BSP_CFG_RTOS == 1 // ThreadX
    .p_semaphore_name = &g_ob1203_sensor0_semaphore_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_semaphore_memory = &g_ob1203_sensor0_semaphore_memory,
#endif
};
#endif

rm_ob1203_instance_ctrl_t g_ob1203_sensor0_ctrl;
const rm_ob1203_cfg_t g_ob1203_sensor0_cfg = {
#if BSP_CFG_RTOS
    .p_semaphore = &g_ob1203_sensor0_semaphore,
#endif
		.semaphore_timeout = 0xFFFFFFFF, .p_comms_instance =
				&g_comms_i2c_device3,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_external_irq14)
    .p_irq_instance = NULL,
    .p_irq_callback = NULL,
#else
		.p_irq_instance = &g_external_irq14, .p_irq_callback =
				ob1203_irq_callback,
#endif
#undef RA_NOT_DEFINED
		.p_comms_callback = ob1203_comms_i2c_callback,
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = (void*) &g_ob1203_sensor0_extended_cfg, };

const rm_ob1203_instance_t g_ob1203_sensor0 = {
		.p_ctrl = &g_ob1203_sensor0_ctrl, .p_cfg = &g_ob1203_sensor0_cfg,
		.p_api = &g_ob1203_on_ob1203, };
extern uint32_t g_fsp_common_thread_count;

const rm_freertos_port_parameters_t oximeter_thread_parameters = { .p_context =
		(void*) NULL, };

void oximeter_thread_create(void) {
	/* Increment count so we will know the number of threads created in the RA Configuration editor. */
	g_fsp_common_thread_count++;

	/* Initialize each kernel object. */

#if 1
	oximeter_thread = xTaskCreateStatic(
#else
                    BaseType_t oximeter_thread_create_err = xTaskCreate(
                    #endif
			oximeter_thread_func, (const char*) "Oximeter Thread", 2048 / 4, // In words, not bytes
			(void*) &oximeter_thread_parameters, //pvParameters
			4,
#if 1
			(StackType_t*) &oximeter_thread_stack,
			(StaticTask_t*) &oximeter_thread_memory
#else
                        & oximeter_thread
                        #endif
			);

#if 1
	if (NULL == oximeter_thread) {
		rtos_startup_err_callback(oximeter_thread, 0);
	}
#else
                    if (pdPASS != oximeter_thread_create_err)
                    {
                        rtos_startup_err_callback(oximeter_thread, 0);
                    }
                    #endif
}
static void oximeter_thread_func(void *pvParameters) {
	/* Initialize common components */
	rtos_startup_common_init();

	/* Initialize each module instance. */

#if (1 == BSP_TZ_NONSECURE_BUILD) && (1 == 1)
                    /* When FreeRTOS is used in a non-secure TrustZone application, portALLOCATE_SECURE_CONTEXT must be called prior
                     * to calling any non-secure callable function in a thread. The parameter is unused in the FSP implementation.
                     * If no slots are available then configASSERT() will be called from vPortSVCHandler_C(). If this occurs, the
                     * application will need to either increase the value of the "Process Stack Slots" Property in the rm_tz_context
                     * module in the secure project or decrease the number of threads in the non-secure project that are allocating
                     * a secure context. Users can control which threads allocate a secure context via the Properties tab when
                     * selecting each thread. Note that the idle thread in FreeRTOS requires a secure context so the application
                     * will need at least 1 secure context even if no user threads make secure calls. */
                     portALLOCATE_SECURE_CONTEXT(0);
                    #endif

	/* Enter user code for this thread. Pass task handle. */
	oximeter_thread_entry(pvParameters);
}
