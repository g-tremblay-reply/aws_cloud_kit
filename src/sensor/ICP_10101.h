/***********************************************************************************************************************
 * File Name    : ICP_10101.h
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

#ifndef ICP_10101_H_
#define ICP_10101_H_

#include "stdbool.h"
#include "hal_data.h"

typedef enum
{ FAST,
NORMAL,
ACCURATE,
VERY_ACCURATE
}mmode;

void timer_start(void);
void timer_reset(void);
uint8_t icp_begin(void);
bool isConnected(void);
void measure(mmode mode);
uint8_t measureStart(mmode mode);
bool dataReady();
float getTemperatureC(void);
float getTemperatureF(void);
float getPressurePa(void);
void ICP_10101_get();
uint8_t begin(void);

void _calculate(void);
void _sendCommand(uint16_t cmd);
void _sendCommands(uint8_t *cmd_buf, uint8_t cmd_len);
void _readResponse(uint8_t *res_buf, uint8_t res_len );

uint32_t GetMilliTick(void);


#endif /* ICP_10101_H_ */
