/* generated thread source file - do not edit */
#include "zmod_thread.h"

#if 1
static StaticTask_t zmod_thread_memory;
#if defined(__ARMCC_VERSION)           /* AC6 compiler */
                static uint8_t zmod_thread_stack[1024] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
                #else
static uint8_t zmod_thread_stack[1024] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.zmod_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
#endif
#endif
TaskHandle_t zmod_thread;
void zmod_thread_create(void);
static void zmod_thread_func(void *pvParameters);
void rtos_startup_err_callback(void *p_instance, void *p_data);
void rtos_startup_common_init(void);
/* I2C Communication Device */
#define RA_NOT_DEFINED (1)
rm_comms_i2c_instance_ctrl_t g_comms_i2c_device1_ctrl;

/* Lower level driver configuration */
const i2c_master_cfg_t g_comms_i2c_device1_lower_level_cfg = { .slave = 0x32,
		.addr_mode = I2C_MASTER_ADDR_MODE_7BIT, };

const rm_comms_cfg_t g_comms_i2c_device1_cfg = {
		.semaphore_timeout = 0xFFFFFFFF, .p_lower_level_cfg =
				(void*) &g_comms_i2c_device1_lower_level_cfg, .p_extend =
				(void*) &g_comms_i2c_bus0_extended_cfg, .p_callback =
				rm_zmod4xxx_comms_i2c_callback,
#if defined(g_zmod4xxx_sensor0_ctrl)
    .p_context          = g_zmod4xxx_sensor0_ctrl,
#else
		.p_context = (void*) &g_zmod4xxx_sensor0_ctrl,
#endif
		};

const rm_comms_instance_t g_comms_i2c_device1 = { .p_ctrl =
		&g_comms_i2c_device1_ctrl, .p_cfg = &g_comms_i2c_device1_cfg, .p_api =
		&g_comms_on_comms_i2c, };
zmod4xxx_dev_t g_zmod4xxx_sensor0_dev;
iaq_1st_gen_handle_t g_zmod4xxx_sensor0_lib_handle;
iaq_1st_gen_results_t g_zmod4xxx_sensor0_lib_results;
uint8_t g_zmod4xxx_sensor0_product_data[6];
extern rm_zmod4xxx_api_t const g_zmod4xxx_on_zmod4410_iaq_1st_gen;
extern zmod4xxx_conf g_zmod4410_iaq_1st_gen_sensor_type[];
rm_zmod4xxx_lib_extended_cfg_t g_zmod4xxx_sensor0_extended_cfg = {
#if (1 == 1) // Continuous mode
		.lib_type = RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_CONTINUOUS,
		.sample_period = 2.0F,
#else                                                   // Low Power mode
    .lib_type = RM_ZMOD4410_LIB_TYPE_IAQ_1ST_GEN_LOW_POWER,
    .sample_period = 6.0F,
#endif
		.product_id = 0x2310, .p_api =
				(void*) &g_zmod4xxx_on_zmod4410_iaq_1st_gen, .p_data_set =
				(void*) g_zmod4410_iaq_1st_gen_sensor_type, .p_product_data =
				g_zmod4xxx_sensor0_product_data, .p_device =
				(void*) &g_zmod4xxx_sensor0_dev, .p_handle =
				(void*) &g_zmod4xxx_sensor0_lib_handle, .p_results =
				(void*) &g_zmod4xxx_sensor0_lib_results, };
rm_zmod4xxx_instance_ctrl_t g_zmod4xxx_sensor0_ctrl;
const rm_zmod4xxx_cfg_t g_zmod4xxx_sensor0_cfg = { .p_comms_instance =
		&g_comms_i2c_device1,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_external_irq0)
    .p_irq_instance = NULL,
    .p_irq_callback = NULL,
#else
		.p_irq_instance = &g_external_irq0, .p_irq_callback =
				SensorOaq_EndOfMeasurement,
#endif
#undef RA_NOT_DEFINED
		.p_comms_callback = SensorOaq_CommCallback,
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = (void*) &g_zmod4xxx_sensor0_extended_cfg, };

const rm_zmod4xxx_instance_t g_zmod4xxx_sensor0 = { .p_ctrl =
		&g_zmod4xxx_sensor0_ctrl, .p_cfg = &g_zmod4xxx_sensor0_cfg, .p_api =
		&g_zmod4xxx_on_zmod4xxx, };
extern uint32_t g_fsp_common_thread_count;

const rm_freertos_port_parameters_t zmod_thread_parameters = { .p_context =
		(void*) NULL, };

void zmod_thread_create(void) {
	/* Increment count so we will know the number of threads created in the RA Configuration editor. */
	g_fsp_common_thread_count++;

	/* Initialize each kernel object. */

#if 1
	zmod_thread = xTaskCreateStatic(
#else
                    BaseType_t zmod_thread_create_err = xTaskCreate(
                    #endif
			zmod_thread_func, (const char*) "Zmod Thread", 1024 / 4, // In words, not bytes
			(void*) &zmod_thread_parameters, //pvParameters
			3,
#if 1
			(StackType_t*) &zmod_thread_stack,
			(StaticTask_t*) &zmod_thread_memory
#else
                        & zmod_thread
                        #endif
			);

#if 1
	if (NULL == zmod_thread) {
		rtos_startup_err_callback(zmod_thread, 0);
	}
#else
                    if (pdPASS != zmod_thread_create_err)
                    {
                        rtos_startup_err_callback(zmod_thread, 0);
                    }
                    #endif
}
static void zmod_thread_func(void *pvParameters) {
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
	zmod_thread_entry(pvParameters);
}
