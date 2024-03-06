/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        #if __has_include("r_ioport.h")
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
            [1] = gpt_counter_overflow_isr, /* GPT2 COUNTER OVERFLOW (Overflow) */
            [2] = sci_uart_rxi_isr, /* SCI5 RXI (Received data full) */
            [3] = sci_uart_txi_isr, /* SCI5 TXI (Transmit data empty) */
            [4] = sci_uart_tei_isr, /* SCI5 TEI (Transmit end) */
            [5] = sci_uart_eri_isr, /* SCI5 ERI (Receive error) */
            [6] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [7] = fcu_fiferr_isr, /* FCU FIFERR (Flash access error interrupt) */
            [8] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [9] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [10] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [11] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
            [12] = r_icu_isr, /* ICU IRQ15 (External pin interrupt 15) */
            [13] = ether_eint_isr, /* EDMAC0 EINT (EDMAC 0 interrupt) */
            [14] = r_icu_isr, /* ICU IRQ14 (External pin interrupt 14) */
            [15] = r_icu_isr, /* ICU IRQ4 (External pin interrupt 4) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_GPT1_COUNTER_OVERFLOW), /* GPT1 COUNTER OVERFLOW (Overflow) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_GPT2_COUNTER_OVERFLOW), /* GPT2 COUNTER OVERFLOW (Overflow) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SCI5_RXI), /* SCI5 RXI (Received data full) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_SCI5_TXI), /* SCI5 TXI (Transmit data empty) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_SCI5_TEI), /* SCI5 TEI (Transmit end) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_SCI5_ERI), /* SCI5 ERI (Receive error) */
            [6] = BSP_PRV_IELS_ENUM(EVENT_FCU_FRDYI), /* FCU FRDYI (Flash ready interrupt) */
            [7] = BSP_PRV_IELS_ENUM(EVENT_FCU_FIFERR), /* FCU FIFERR (Flash access error interrupt) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_IIC0_RXI), /* IIC0 RXI (Receive data full) */
            [9] = BSP_PRV_IELS_ENUM(EVENT_IIC0_TXI), /* IIC0 TXI (Transmit data empty) */
            [10] = BSP_PRV_IELS_ENUM(EVENT_IIC0_TEI), /* IIC0 TEI (Transmit end) */
            [11] = BSP_PRV_IELS_ENUM(EVENT_IIC0_ERI), /* IIC0 ERI (Transfer error) */
            [12] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ15), /* ICU IRQ15 (External pin interrupt 15) */
            [13] = BSP_PRV_IELS_ENUM(EVENT_EDMAC0_EINT), /* EDMAC0 EINT (EDMAC 0 interrupt) */
            [14] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ14), /* ICU IRQ14 (External pin interrupt 14) */
            [15] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ4), /* ICU IRQ4 (External pin interrupt 4) */
        };
        #elif __has_include("r_ioport_b.h")
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_IRQ_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
            [BSP_PRV_IELS_ENUM(GPT1_COUNTER_OVERFLOW)] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
            [BSP_PRV_IELS_ENUM(GPT2_COUNTER_OVERFLOW)] = gpt_counter_overflow_isr, /* GPT2 COUNTER OVERFLOW (Overflow) */
            [BSP_PRV_IELS_ENUM(SCI5_RXI)] = sci_uart_rxi_isr, /* SCI5 RXI (Received data full) */
            [BSP_PRV_IELS_ENUM(SCI5_TXI)] = sci_uart_txi_isr, /* SCI5 TXI (Transmit data empty) */
            [BSP_PRV_IELS_ENUM(SCI5_TEI)] = sci_uart_tei_isr, /* SCI5 TEI (Transmit end) */
            [BSP_PRV_IELS_ENUM(SCI5_ERI)] = sci_uart_eri_isr, /* SCI5 ERI (Receive error) */
            [BSP_PRV_IELS_ENUM(FCU_FRDYI)] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [BSP_PRV_IELS_ENUM(FCU_FIFERR)] = fcu_fiferr_isr, /* FCU FIFERR (Flash access error interrupt) */
            [BSP_PRV_IELS_ENUM(IIC0_RXI)] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [BSP_PRV_IELS_ENUM(IIC0_TXI)] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [BSP_PRV_IELS_ENUM(IIC0_TEI)] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [BSP_PRV_IELS_ENUM(IIC0_ERI)] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
            [BSP_PRV_IELS_ENUM(ICU_IRQ15)] = r_icu_isr, /* ICU IRQ15 (External pin interrupt 15) */
            [BSP_PRV_IELS_ENUM(EDMAC0_EINT)] = ether_eint_isr, /* EDMAC0 EINT (EDMAC 0 interrupt) */
            [BSP_PRV_IELS_ENUM(ICU_IRQ14)] = r_icu_isr, /* ICU IRQ14 (External pin interrupt 14) */
            [BSP_PRV_IELS_ENUM(ICU_IRQ4)] = r_icu_isr, /* ICU IRQ4 (External pin interrupt 4) */
        };
        #endif
        #endif
