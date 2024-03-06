/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : common_init.c
 * Description  : Common init function.
 **********************************************************************************************************************/

#include "common_init.h"
#include "r_typedefs.h"

#define NUM_RATES             (sizeof(pwm_rates) / sizeof(pwm_rates[0]))   /*  */
#define NUM_DCS               (sizeof(pwm_dcs) / sizeof(pwm_dcs[0]))       /*  */
#define NUM_SWITCH            (sizeof(s_irq_pins) / sizeof(s_irq_pins[0])) /*  */

typedef struct irq_pins
{
    const external_irq_instance_t * const p_irq;
} st_irq_pins_t;


int32_t g_curr_led_freq = BLINK_FREQ_1HZ;

static st_irq_pins_t s_irq_pins[] =
{
     { &g_external_irq1 }
};


/*******************************************************************************************************************//**
 * @brief    LED initialization
 * @param[in]
 * @retval   FSP_SUCCESS
 ***********************************************************************************************************************/

static fsp_err_t led_initialize(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    R_BSP_PinAccessEnable();

    /* LED1,3,4 should be 'off' durning power up*/

    R_BSP_PinAccessDisable();

    return fsp_err;
}

/*******************************************************************************************************************//**
 * @brief    ICU initialization
 * @param[in]
 * @retval   FSP_SUCCESS or Error Code
 ***********************************************************************************************************************/

static fsp_err_t icu_initialize(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    for (uint32_t i = 0; i < NUM_SWITCH; i++ )
    {
        fsp_err = R_ICU_ExternalIrqOpen (s_irq_pins[i].p_irq->p_ctrl, s_irq_pins[i].p_irq->p_cfg);
        if (FSP_SUCCESS != fsp_err)
        {
            return fsp_err;
        }

        fsp_err = R_ICU_ExternalIrqEnable (s_irq_pins[i].p_irq->p_ctrl);
        if (FSP_SUCCESS != fsp_err)
        {
            return fsp_err;
        }
    }
    return fsp_err;
}

/*******************************************************************************************************************//**
 * @brief    Common Init containing 'led_initialize' and 'icu_initialize' func calls
 * @param[in]
 * @retval   FSP_SUCCESS or Error Code
 ***********************************************************************************************************************/

fsp_err_t common_init(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = led_initialize ();
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    fsp_err = icu_initialize ();
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    /* Set baseline LED status */
    g_board_status.led_intensity = 0;
    g_board_status.led_frequency = 0;

    return fsp_err;
}
/**********************************************************************************************************************
 End of function common_init
 *********************************************************************************************************************/

/**********************************************************************************************************************
 End of function periodic_timer_callback
 *********************************************************************************************************************/
