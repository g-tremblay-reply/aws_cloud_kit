/* generated common source file - do not edit */
#include "common_data.h"

icu_instance_ctrl_t g_external_irq0_ctrl;
const external_irq_cfg_t g_external_irq0_cfg = { .channel = 4, .trigger =
		EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .clock_source_div =
		EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64,
		.p_callback = rm_zmod4xxx_irq_callback,
		/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = NULL, .ipl = (3),
#if defined(VECTOR_NUMBER_ICU_IRQ4)
    .irq                 = VECTOR_NUMBER_ICU_IRQ4,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq0 = { .p_ctrl =
		&g_external_irq0_ctrl, .p_cfg = &g_external_irq0_cfg, .p_api =
		&g_external_irq_on_icu };
icu_instance_ctrl_t g_external_irq14_ctrl;
const external_irq_cfg_t g_external_irq14_cfg = { .channel = 14, .trigger =
		EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .clock_source_div =
		EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64, .p_callback = rm_ob1203_irq_callback,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = NULL, .ipl = (12),
#if defined(VECTOR_NUMBER_ICU_IRQ14)
    .irq                 = VECTOR_NUMBER_ICU_IRQ14,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq14 = { .p_ctrl =
		&g_external_irq14_ctrl, .p_cfg = &g_external_irq14_cfg, .p_api =
		&g_external_irq_on_icu };
#ifndef vLoggingPrintf
#include <stdarg.h>

void vLoggingPrintf(const char *pcFormat, ...) {
	FSP_PARAMETER_NOT_USED(pcFormat);
}
#endif

#ifndef vLoggingPrint
void vLoggingPrint(const char *pcFormat);

void vLoggingPrint(const char *pcFormat) {
	FSP_PARAMETER_NOT_USED(pcFormat);
}
#endif
flash_hp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg = { .data_flash_bgo = false, .p_callback = NULL,
		.p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
		.err_irq = FSP_INVALID_VECTOR,
#endif
		.err_ipl = (BSP_IRQ_DISABLED), .ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const flash_instance_t g_flash0 = { .p_ctrl = &g_flash0_ctrl, .p_cfg =
		&g_flash0_cfg, .p_api = &g_flash_on_flash_hp };
rm_littlefs_flash_instance_ctrl_t g_rm_littlefs0_ctrl;

#ifdef LFS_NO_MALLOC
static uint8_t g_rm_littlefs0_read[64];
static uint8_t g_rm_littlefs0_prog[64];
static uint8_t g_rm_littlefs0_lookahead[16];
#endif

struct lfs g_rm_littlefs0_lfs;

const struct lfs_config g_rm_littlefs0_lfs_cfg = { .context =
		&g_rm_littlefs0_ctrl, .read = &rm_littlefs_flash_read, .prog =
		&rm_littlefs_flash_write, .erase = &rm_littlefs_flash_erase, .sync =
		&rm_littlefs_flash_sync, .read_size = 1, .prog_size = 4, .block_size =
		128, .block_count = (BSP_DATA_FLASH_SIZE_BYTES / 256), .block_cycles =
		1024, .cache_size = 64, .lookahead_size = 16,
#ifdef LFS_NO_MALLOC
    .read_buffer = (void *) g_rm_littlefs0_read,
    .prog_buffer = (void *) g_rm_littlefs0_prog,
    .lookahead_buffer = (void *) g_rm_littlefs0_lookahead,
#endif
#if LFS_THREAD_SAFE
    .lock   = &rm_littlefs_flash_lock,
    .unlock = &rm_littlefs_flash_unlock,
#endif
		};

const rm_littlefs_flash_cfg_t g_rm_littlefs0_ext_cfg = { .p_flash = &g_flash0, };

const rm_littlefs_cfg_t g_rm_littlefs0_cfg = { .p_lfs_cfg =
		&g_rm_littlefs0_lfs_cfg, .p_extend = &g_rm_littlefs0_ext_cfg, };

/* Instance structure to use this module. */
const rm_littlefs_instance_t g_rm_littlefs0 = { .p_ctrl = &g_rm_littlefs0_ctrl,
		.p_cfg = &g_rm_littlefs0_cfg, .p_api = &g_rm_littlefs_on_flash, };

ether_phy_instance_ctrl_t g_ether_phy0_ctrl;

const ether_phy_extended_cfg_t g_ether_phy0_extended_cfg = { .p_target_init =
		NULL, .p_target_link_partner_ability_get = NULL

};

const ether_phy_cfg_t g_ether_phy0_cfg = {

.channel = 0, .phy_lsi_address = 5, .phy_reset_wait_time = 0x00020000,
		.mii_bit_access_wait_time = 8, .phy_lsi_type =
				ETHER_PHY_LSI_TYPE_KIT_COMPONENT, .flow_control =
				ETHER_PHY_FLOW_CONTROL_DISABLE, .mii_type =
				ETHER_PHY_MII_TYPE_RMII, .p_context = NULL, .p_extend =
				&g_ether_phy0_extended_cfg,

};
/* Instance structure to use this module. */
const ether_phy_instance_t g_ether_phy0 = { .p_ctrl = &g_ether_phy0_ctrl,
		.p_cfg = &g_ether_phy0_cfg, .p_api = &g_ether_phy_on_ether_phy };
ether_instance_ctrl_t g_ether0_ctrl;

uint8_t g_ether0_mac_address[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x57 };

__attribute__((__aligned__(16))) ether_instance_descriptor_t g_ether0_tx_descriptors[8] ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(16))) ether_instance_descriptor_t g_ether0_rx_descriptors[8] ETHER_BUFFER_PLACE_IN_SECTION;

__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer0[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer1[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer2[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer3[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer4[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer5[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer6[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer7[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer8[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer9[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer10[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer11[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer12[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer13[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer14[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer15[1536]ETHER_BUFFER_PLACE_IN_SECTION;

uint8_t *pp_g_ether0_ether_buffers[16] = {
		(uint8_t*) &g_ether0_ether_buffer0[0],
		(uint8_t*) &g_ether0_ether_buffer1[0],
		(uint8_t*) &g_ether0_ether_buffer2[0],
		(uint8_t*) &g_ether0_ether_buffer3[0],
		(uint8_t*) &g_ether0_ether_buffer4[0],
		(uint8_t*) &g_ether0_ether_buffer5[0],
		(uint8_t*) &g_ether0_ether_buffer6[0],
		(uint8_t*) &g_ether0_ether_buffer7[0],
		(uint8_t*) &g_ether0_ether_buffer8[0],
		(uint8_t*) &g_ether0_ether_buffer9[0],
		(uint8_t*) &g_ether0_ether_buffer10[0],
		(uint8_t*) &g_ether0_ether_buffer11[0],
		(uint8_t*) &g_ether0_ether_buffer12[0],
		(uint8_t*) &g_ether0_ether_buffer13[0],
		(uint8_t*) &g_ether0_ether_buffer14[0],
		(uint8_t*) &g_ether0_ether_buffer15[0], };

const ether_extended_cfg_t g_ether0_extended_cfg_t = { .p_rx_descriptors =
		g_ether0_rx_descriptors, .p_tx_descriptors = g_ether0_tx_descriptors, };

const ether_cfg_t g_ether0_cfg = { .channel = 0, .zerocopy =
		ETHER_ZEROCOPY_DISABLE, .multicast = ETHER_MULTICAST_ENABLE,
		.promiscuous = ETHER_PROMISCUOUS_DISABLE, .flow_control =
				ETHER_FLOW_CONTROL_DISABLE, .padding = ETHER_PADDING_DISABLE,
		.padding_offset = 0, .broadcast_filter = 0, .p_mac_address =
				g_ether0_mac_address,

		.num_tx_descriptors = 8, .num_rx_descriptors = 8,

		.pp_ether_buffers = pp_g_ether0_ether_buffers,

		.ether_buffer_size = 1536,

#if defined(VECTOR_NUMBER_EDMAC0_EINT)
                .irq                = VECTOR_NUMBER_EDMAC0_EINT,
#else
		.irq = FSP_INVALID_VECTOR,
#endif

		.interrupt_priority = (5),

		.p_callback = vEtherISRCallback, .p_ether_phy_instance = &g_ether_phy0,
		.p_context = NULL, .p_extend = &g_ether0_extended_cfg_t, };

/* Instance structure to use this module. */
const ether_instance_t g_ether0 = { .p_ctrl = &g_ether0_ctrl, .p_cfg =
		&g_ether0_cfg, .p_api = &g_ether_on_ether };
ether_instance_t const *gp_freertos_ether = &g_ether0;
icu_instance_ctrl_t g_external_irq1_ctrl;
const external_irq_cfg_t g_external_irq1_cfg = { .channel = 15, .trigger =
		EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .clock_source_div =
		EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64,
		.p_callback = rm_zmod4xxx_irq_callback,
		/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = NULL, .ipl = (12),
#if defined(VECTOR_NUMBER_ICU_IRQ15)
    .irq                 = VECTOR_NUMBER_ICU_IRQ15,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq1 = { .p_ctrl =
		&g_external_irq1_ctrl, .p_cfg = &g_external_irq1_cfg, .p_api =
		&g_external_irq_on_icu };
iic_master_instance_ctrl_t g_i2c_master0_ctrl;
const iic_master_extended_cfg_t g_i2c_master0_extend =
		{ .timeout_mode = IIC_MASTER_TIMEOUT_MODE_SHORT, .timeout_scl_low =
				IIC_MASTER_TIMEOUT_SCL_LOW_ENABLED,
				/* Actual calculated bitrate: 396825. Actual calculated duty cycle: 51%. */.clock_settings.brl_value =
						25, .clock_settings.brh_value = 26,
				.clock_settings.cks_value = 1, };
const i2c_master_cfg_t g_i2c_master0_cfg = { .channel = 0, .rate =
		I2C_MASTER_RATE_FAST, .slave = 0x00, .addr_mode =
		I2C_MASTER_ADDR_MODE_7BIT,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
		.p_callback = rm_comms_i2c_callback, .p_context = NULL,
#if defined(VECTOR_NUMBER_IIC0_RXI)
    .rxi_irq             = VECTOR_NUMBER_IIC0_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_IIC0_TXI)
    .txi_irq             = VECTOR_NUMBER_IIC0_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_IIC0_TEI)
    .tei_irq             = VECTOR_NUMBER_IIC0_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_IIC0_ERI)
    .eri_irq             = VECTOR_NUMBER_IIC0_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		.ipl = (5), .p_extend = &g_i2c_master0_extend, };
/* Instance structure to use this module. */
const i2c_master_instance_t g_i2c_master0 = { .p_ctrl = &g_i2c_master0_ctrl,
		.p_cfg = &g_i2c_master0_cfg, .p_api = &g_i2c_master_on_iic };
#if BSP_CFG_RTOS
#if BSP_CFG_RTOS == 1
#if !defined(g_comms_i2c_bus0_recursive_mutex)
TX_MUTEX g_comms_i2c_bus0_recursive_mutex_handle;
CHAR g_comms_i2c_bus0_recursive_mutex_name[] = "g_comms_i2c_bus0 recursive mutex";
#endif
#if !defined(g_comms_i2c_bus0_blocking_semaphore)
TX_SEMAPHORE g_comms_i2c_bus0_blocking_semaphore_handle;
CHAR g_comms_i2c_bus0_blocking_semaphore_name[] = "g_comms_i2c_bus0 blocking semaphore";
#endif
#elif BSP_CFG_RTOS == 2
#if !defined(g_comms_i2c_bus0_recursive_mutex)
SemaphoreHandle_t g_comms_i2c_bus0_recursive_mutex_handle;
StaticSemaphore_t g_comms_i2c_bus0_recursive_mutex_memory;
#endif
#if !defined(g_comms_i2c_bus0_blocking_semaphore)
SemaphoreHandle_t g_comms_i2c_bus0_blocking_semaphore_handle;
StaticSemaphore_t g_comms_i2c_bus0_blocking_semaphore_memory;
#endif
#endif

#if !defined(g_comms_i2c_bus0_recursive_mutex)
/* Recursive Mutex for I2C bus */
rm_comms_i2c_mutex_t g_comms_i2c_bus0_recursive_mutex =
{
    .p_mutex_handle = &g_comms_i2c_bus0_recursive_mutex_handle,
#if BSP_CFG_RTOS == 1 // ThradX
    .p_mutex_name = &g_comms_i2c_bus0_recursive_mutex_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_mutex_memory = &g_comms_i2c_bus0_recursive_mutex_memory,
#endif
};
#endif

#if !defined(g_comms_i2c_bus0_blocking_semaphore)
/* Semaphore for blocking */
rm_comms_i2c_semaphore_t g_comms_i2c_bus0_blocking_semaphore =
{
    .p_semaphore_handle = &g_comms_i2c_bus0_blocking_semaphore_handle,
#if BSP_CFG_RTOS == 1 // ThreadX
    .p_semaphore_name = &g_comms_i2c_bus0_blocking_semaphore_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_semaphore_memory = &g_comms_i2c_bus0_blocking_semaphore_memory,
#endif
};
#endif
#endif

/* Shared I2C Bus */
#define RA_NOT_DEFINED (1)
rm_comms_i2c_bus_extended_cfg_t g_comms_i2c_bus0_extended_cfg = {
#if !defined(g_i2c_master0)
		.p_driver_instance = (void*) &g_i2c_master0,
#elif !defined(RA_NOT_DEFINED)
    .p_driver_instance      = (void*)&RA_NOT_DEFINED,
#endif
		.p_current_ctrl = NULL, .bus_timeout = 0xFFFFFFFF,
#if BSP_CFG_RTOS
#if !defined(g_comms_i2c_bus0_blocking_semaphore)
    .p_blocking_semaphore = &g_comms_i2c_bus0_blocking_semaphore,
#if !defined(g_comms_i2c_bus0_recursive_mutex)
    .p_bus_recursive_mutex = &g_comms_i2c_bus0_recursive_mutex,
#else
    .p_bus_recursive_mutex = NULL,
#endif
#else
    .p_bus_recursive_mutex = NULL,
    .p_blocking_semaphore = NULL,
#endif
#else
#endif
		};
#if BSP_FEATURE_CGC_HAS_OSTDCSE
const cgc_extended_cfg_t g_cgc0_cfg_extend =
{
    .ostd_enable = (1),
    .mostd_enable = (0),
    .sostd_enable = (0),
#if defined(VECTOR_NUMBER_FIXED_MOSTD_STOP)
    .mostd_irq            = VECTOR_NUMBER_FIXED_MOSTD_STOP,
#else
    .mostd_irq            = FSP_INVALID_VECTOR,
#endif
    .mostd_ipl            = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_FIXED_SOSC_STOP)
    .sostd_irq            = VECTOR_NUMBER_FIXED_SOSC_STOP,
#else
    .sostd_irq            = FSP_INVALID_VECTOR,
#endif
    .sostd_ipl            = (BSP_IRQ_DISABLED),
    .sdadc_b_clock_switch_enable = (0),
    .mostd_detection_time = 0,
    .sostd_detection_time = 0,
};
#endif

#if BSP_TZ_NONSECURE_BUILD
 #if defined(BSP_CFG_CLOCKS_SECURE) && BSP_CFG_CLOCKS_SECURE
  #error "The CGC registers are only accessible in the TrustZone Secure Project."
 #endif
#endif

const cgc_cfg_t g_cgc0_cfg = { .p_callback = NULL,
#if BSP_FEATURE_CGC_HAS_OSTDCSE
    .p_extend   = &g_cgc0_cfg_extend,
#else
		.p_extend = NULL,
#endif
		};

cgc_instance_ctrl_t g_cgc0_ctrl;
const cgc_instance_t g_cgc0 = { .p_api = &g_cgc_on_cgc, .p_ctrl = &g_cgc0_ctrl,
		.p_cfg = &g_cgc0_cfg, };
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport = { .p_api = &g_ioport_on_ioport, .p_ctrl =
		&g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };

QueueHandle_t g_topic_queue;
#if 1
StaticQueue_t g_topic_queue_memory;
uint8_t g_topic_queue_queue_memory[65 * 16];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_sens_data_mutex;
#if 1
StaticSemaphore_t g_sens_data_mutex_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_console_binary_semaphore;
#if 1
StaticSemaphore_t g_console_binary_semaphore_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_console_out_mutex;
#if 1
StaticSemaphore_t g_console_out_mutex_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_update_console_event;
#if 1
StaticSemaphore_t g_update_console_event_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
SemaphoreHandle_t g_ob1203_semaphore;
#if 1
StaticSemaphore_t g_ob1203_semaphore_memory;
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_ob1203_queue;
#if 1
StaticQueue_t g_ob1203_queue_memory;
uint8_t g_ob1203_queue_queue_memory[10 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_hs3001_queue;
#if 1
StaticQueue_t g_hs3001_queue_memory;
uint8_t g_hs3001_queue_queue_memory[8 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_iaq_queue;
#if 1
StaticQueue_t g_iaq_queue_memory;
uint8_t g_iaq_queue_queue_memory[12 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_oaq_queue;
#if 1
StaticQueue_t g_oaq_queue_memory;
uint8_t g_oaq_queue_queue_memory[4 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_icm_queue;
#if 1
StaticQueue_t g_icm_queue_memory;
uint8_t g_icm_queue_queue_memory[72 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
QueueHandle_t g_icp_queue;
#if 1
StaticQueue_t g_icp_queue_memory;
uint8_t g_icp_queue_queue_memory[16 * 1];
#endif
void rtos_startup_err_callback(void *p_instance, void *p_data);
void g_common_init(void) {
	g_topic_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					16, 65
#if 1
					, &g_topic_queue_queue_memory[0], &g_topic_queue_memory
#endif
					);
	if (NULL == g_topic_queue) {
		rtos_startup_err_callback(g_topic_queue, 0);
	}
	g_sens_data_mutex =
#if 0
                #if 1
                xSemaphoreCreateRecursiveMutexStatic(&g_sens_data_mutex_memory);
                #else
                xSemaphoreCreateRecursiveMutex();
                #endif
                #else
#if 1
			xSemaphoreCreateMutexStatic(&g_sens_data_mutex_memory);
#else
                xSemaphoreCreateMutex();
                #endif
#endif
	if (NULL == g_sens_data_mutex) {
		rtos_startup_err_callback(g_sens_data_mutex, 0);
	}
	g_console_binary_semaphore =
#if 1
			xSemaphoreCreateBinaryStatic(&g_console_binary_semaphore_memory);
#else
                xSemaphoreCreateBinary();
                #endif
	if (NULL == g_console_binary_semaphore) {
		rtos_startup_err_callback(g_console_binary_semaphore, 0);
	}
	g_console_out_mutex =
#if 0
                #if 1
                xSemaphoreCreateRecursiveMutexStatic(&g_console_out_mutex_memory);
                #else
                xSemaphoreCreateRecursiveMutex();
                #endif
                #else
#if 1
			xSemaphoreCreateMutexStatic(&g_console_out_mutex_memory);
#else
                xSemaphoreCreateMutex();
                #endif
#endif
	if (NULL == g_console_out_mutex) {
		rtos_startup_err_callback(g_console_out_mutex, 0);
	}
	g_update_console_event =
#if 0
                #if 1
                xSemaphoreCreateRecursiveMutexStatic(&g_update_console_event_memory);
                #else
                xSemaphoreCreateRecursiveMutex();
                #endif
                #else
#if 1
			xSemaphoreCreateMutexStatic(&g_update_console_event_memory);
#else
                xSemaphoreCreateMutex();
                #endif
#endif
	if (NULL == g_update_console_event) {
		rtos_startup_err_callback(g_update_console_event, 0);
	}
	g_ob1203_semaphore =
#if 1
			xSemaphoreCreateBinaryStatic(&g_ob1203_semaphore_memory);
#else
                xSemaphoreCreateBinary();
                #endif
	if (NULL == g_ob1203_semaphore) {
		rtos_startup_err_callback(g_ob1203_semaphore, 0);
	}
	g_ob1203_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 10
#if 1
					, &g_ob1203_queue_queue_memory[0], &g_ob1203_queue_memory
#endif
					);
	if (NULL == g_ob1203_queue) {
		rtos_startup_err_callback(g_ob1203_queue, 0);
	}
	g_hs3001_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 8
#if 1
					, &g_hs3001_queue_queue_memory[0], &g_hs3001_queue_memory
#endif
					);
	if (NULL == g_hs3001_queue) {
		rtos_startup_err_callback(g_hs3001_queue, 0);
	}
	g_iaq_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 12
#if 1
					, &g_iaq_queue_queue_memory[0], &g_iaq_queue_memory
#endif
					);
	if (NULL == g_iaq_queue) {
		rtos_startup_err_callback(g_iaq_queue, 0);
	}
	g_oaq_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 4
#if 1
					, &g_oaq_queue_queue_memory[0], &g_oaq_queue_memory
#endif
					);
	if (NULL == g_oaq_queue) {
		rtos_startup_err_callback(g_oaq_queue, 0);
	}
	g_icm_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 72
#if 1
					, &g_icm_queue_queue_memory[0], &g_icm_queue_memory
#endif
					);
	if (NULL == g_icm_queue) {
		rtos_startup_err_callback(g_icm_queue, 0);
	}
	g_icp_queue =
#if 1
			xQueueCreateStatic(
#else
                xQueueCreate(
                #endif
					1, 16
#if 1
					, &g_icp_queue_queue_memory[0], &g_icp_queue_memory
#endif
					);
	if (NULL == g_icp_queue) {
		rtos_startup_err_callback(g_icp_queue, 0);
	}
}
